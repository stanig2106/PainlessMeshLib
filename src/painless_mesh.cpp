#include <algorithm>
#include <boost/algorithm/hex.hpp>
#include <boost/filesystem.hpp>
#include "painless_mesh.h"

#include "Arduino.h"

#include "painlessmesh/mesh.hpp"
#include "painlessmesh/protocol.hpp"

using namespace painlessmesh;

#include "painlessmesh/connection.hpp"

painlessmesh::logger::LogClass Log;

#undef F
#include <boost/program_options.hpp>
#define F(string_literal) string_literal
namespace po = boost::program_options;

#include "ota.hpp"

#include <thread>
#include <utility>


PainlessMeshLib::PainlessMeshLib(int port, std::string ota_dir)
    : port(port), ota_dir(std::move(ota_dir)) {
  start_server();
}

void PainlessMeshLib::start_server() {
  stopServer = false;
  Scheduler scheduler;
  boost::asio::io_service io_service;
  Log.setLogLevel((1 << 12) - 1);

  mesh.init(&scheduler, 0);
  std::shared_ptr<AsyncServer> pServer;

  pServer = std::make_shared<AsyncServer>(io_service, port);
  painlessmesh::tcp::initServer<painlessmesh::Connection, painlessMesh>(
      *pServer, mesh);

  mesh.onReceive([this](uint32_t nodeId, const std::string& msg) {
    std::lock_guard<std::mutex> lock(messageCallbacksMutex);
    for (const auto& callback : messageCallbacks) {
      callback(nodeId, msg);
    }
  });

  mesh.onNewConnection([this](uint32_t nodeId) {
    std::lock_guard<std::mutex> lock(connectionCallbacksMutex);
    for (const auto& callback : connectionCallbacks) {
      callback(nodeId);
    }
  });

  setup_ota_watch();

  serverThread = std::thread([this, &io_service]() {
    while (!stopServer) {
      usleep(1000);
      mesh.update();
      io_service.poll();
    }
  });
}

void PainlessMeshLib::stop_server() {
  stopServer = true;
  if (serverThread.joinable()) serverThread.join();
}

void PainlessMeshLib::send_message(uint32_t target, const std::string& message) {
  mesh.sendSingle(target, message);
}

void PainlessMeshLib::add_message_callback(
    const std::function<void(uint32_t, const std::string&)>& callback) {
  std::lock_guard<std::mutex> lock(messageCallbacksMutex);
  messageCallbacks.push_back(callback);
}

void PainlessMeshLib::add_connection_callback(const std::function<void(uint32_t)>& callback) {
  std::lock_guard<std::mutex> lock(connectionCallbacksMutex);
  connectionCallbacks.push_back(callback);
}

void PainlessMeshLib::setup_ota_watch() {
  using namespace painlessmesh::plugin;
  auto files = std::make_shared<std::map<std::string, std::string>>();

  auto task = mesh.addTask(TASK_SECOND, TASK_FOREVER, [this, &files]() {
    check_directory_for_changes(files);
  });

  mesh.onPackage(11, [this, &files](protocol::Variant variant) {
    reply_to_data_requests(files, std::move(variant));
    return true;
  });
}

void PainlessMeshLib::check_directory_for_changes(
    const std::shared_ptr<std::map<std::string, std::string>>& files) {
  boost::filesystem::path p(ota_dir);
  boost::filesystem::directory_iterator end_itr;
  for (boost::filesystem::directory_iterator itr(p); itr != end_itr; ++itr) {
    if (!boost::filesystem::is_regular_file(itr->path())) continue;
    auto stat = addFile(files, itr->path(), TASK_SECOND);
    if (stat.newFile) announce_new_file(files, stat);
  }
}

void PainlessMeshLib::announce_new_file(
    const std::shared_ptr<std::map<std::string, std::string>>& files,
    const FileStat& stat) {
  using namespace painlessmesh::plugin;

  ota::Announce announce;
  announce.md5 = stat.md5;
  announce.role = stat.role;
  announce.hardware = stat.hw;
  announce.noPart =
      ceil(((float)files->operator[](stat.md5).length()) / OTA_PART_SIZE);
  announce.from = mesh.getNodeId();

  auto announceTask = mesh.addTask(
      TASK_MINUTE, 60, [this, &announce]() { mesh.sendPackage(&announce); });
  announceTask->setOnDisable(
      [files, md5 = stat.md5]() { files->erase(md5); });
}

void PainlessMeshLib::reply_to_data_requests(
    const std::shared_ptr<std::map<std::string, std::string>>& files,
    protocol::Variant variant) {
  using namespace painlessmesh::plugin;

  auto pkg = variant.to<ota::DataRequest>();
  if (files->count(pkg.md5)) {
    auto reply =
        ota::Data::replyTo(pkg,
                           files->operator[](pkg.md5).substr(
                               OTA_PART_SIZE * pkg.partNo, OTA_PART_SIZE),
                           pkg.partNo);
    mesh.sendPackage(&reply);
  } else
    Log(ERROR, "File not found");
}
