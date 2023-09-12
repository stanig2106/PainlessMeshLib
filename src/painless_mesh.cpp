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

#include "ota.h"

#include <thread>
#include <utility>


PainlessMeshLib::PainlessMeshLib(int port, std::string ota_dir)
        : port(port), ota_dir(std::move(ota_dir)) {}

void PainlessMeshLib::start_server() {
    stopServer = false;

//    Log.setLogLevel((1 << 12) - 1);
    Log.setLogLevel(0);

    mesh.init(&scheduler, 1);

    pServer = std::make_shared<AsyncServer>(io_service, port);
    painlessmesh::tcp::initServer<painlessmesh::Connection, painlessMesh>(
            *pServer, mesh);

    mesh.onReceive([this](uint32_t nodeId, const std::string &msg) {
        std::lock_guard<std::mutex> lock(messageCallbacksMutex);
        auto it = messageCallbacks.begin();
        while (it != messageCallbacks.end()) {
            if ((*it)(nodeId, msg)) {
                it = messageCallbacks.erase(it);
            } else {
                ++it;
            }
        }
    });

    mesh.onNewConnection([this](uint32_t nodeId) {
        std::lock_guard<std::mutex> lock(connectionCallbacksMutex);
        for (const auto &callback: connectionCallbacks) {
            callback(nodeId);
        }
    });

    setup_ota_watch();

    serverThread = std::thread([this]() {
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

void PainlessMeshLib::join_server() {
    if (serverThread.joinable())
        serverThread.join();
}

std::list<uint32_t> PainlessMeshLib::get_connected_nodes() {
    return mesh.getNodeList();
}

void PainlessMeshLib::send_message(uint32_t target, const std::string &message) {
    mesh.sendSingle(target, message);
}

void PainlessMeshLib::send_message_to_all(const std::string &message) {
    mesh.sendBroadcast(message);
}



void PainlessMeshLib::add_message_callback(
        const std::function<bool(uint32_t, const std::string &)> &callback) {
    std::lock_guard<std::mutex> lock(messageCallbacksMutex);
    messageCallbacks.push_back(callback);
}

void PainlessMeshLib::add_connection_callback(const std::function<void(uint32_t)> &callback) {
    std::lock_guard<std::mutex> lock(connectionCallbacksMutex);
    connectionCallbacks.push_back(callback);
}

void PainlessMeshLib::setup_ota_watch() {
    using namespace painlessmesh::plugin;
    files = std::make_shared<std::map<std::string, std::string>>();
    mesh.addTask(TASK_SECOND, TASK_FOREVER, [this]() {
        boost::filesystem::path p(ota_dir);
        boost::filesystem::directory_iterator end_itr;
        for (boost::filesystem::directory_iterator itr(p); itr != end_itr; ++itr) {
            if (!boost::filesystem::is_regular_file(itr->path())) {
                continue;
            }
            auto stat = addFile(files, itr->path(), TASK_SECOND);
            if (stat.newFile) {
                // When change, announce it, load it into files
                ota::Announce announce;
                announce.md5 = stat.md5;
                announce.role = stat.role;
                announce.hardware = stat.hw;
                announce.noPart =
                        ceil(((float) files->operator[](stat.md5).length()) / OTA_PART_SIZE);
                announce.from = mesh.getNodeId();

                auto announceTask = mesh.addTask(TASK_MINUTE, 60, [this, announce]() {
                    mesh.sendPackage(&announce);
                });
                announceTask->setOnDisable(
                        [this, md5 = stat.md5]() { files->erase(md5); });
            }
        }
    });
    mesh.onPackage(11, [this](protocol::Variant variant) {
        auto pkg = variant.to<ota::DataRequest>();
        if (files->count(pkg.md5)) {
            auto reply =
                    ota::Data::replyTo(pkg,
                                       files->operator[](pkg.md5).substr(
                                               OTA_PART_SIZE * pkg.partNo, OTA_PART_SIZE),
                                       pkg.partNo);
            mesh.sendPackage(&reply);
        } else {
            Log(ERROR, "File not found");
        }
        return true;
    });
}