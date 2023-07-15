#ifndef LIBPAINLESSMESHBOOST_DYLIB_SRC_LIB_H_
#define LIBPAINLESSMESHBOOST_DYLIB_SRC_LIB_H_

#include <algorithm>
#include <boost/algorithm/hex.hpp>
#include <boost/filesystem.hpp>

#include "Arduino.h"

#include "painlessmesh/mesh.hpp"
#include "painlessmesh/protocol.hpp"

#include "painlessMesh.h"
#include "painlessmesh/connection.hpp"

#include <thread>
#include <utility>

constexpr uint32_t OTA_PART_SIZE = 1024;
#include "ota.hpp"

class PainlessMeshLib {
 private:
  std::thread serverThread;
  bool stopServer{};
  painlessMesh mesh;
  int port;
  const std::string ota_dir;
  std::vector<std::function<void(uint32_t, const std::string&)>>
      messageCallbacks;
  std::vector<std::function<void(uint32_t)>> connectionCallbacks;
  std::mutex messageCallbacksMutex;
  std::mutex connectionCallbacksMutex;

 public:
  explicit PainlessMeshLib(int port, std::string ota_dir);
  void start_server();
  void stop_server();
  void send_message(uint32_t target, const std::string& message);
  void add_message_callback(
      const std::function<void(uint32_t, const std::string&)>& callback);
  void add_connection_callback(const std::function<void(uint32_t)>& callback);

 private:
  void setup_ota_watch();
  void check_directory_for_changes(
      const std::shared_ptr<std::map<std::string, std::string>>& files);
  void announce_new_file(
      const std::shared_ptr<std::map<std::string, std::string>>& files,
      const FileStat& stat);
  void reply_to_data_requests(
      const std::shared_ptr<std::map<std::string, std::string>>& files,
      painlessmesh::protocol::Variant variant);
};

#endif  // LIBPAINLESSMESHBOOST_DYLIB_SRC_LIB_H_
