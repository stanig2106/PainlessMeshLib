#ifndef _PMBOOST_OTA_H_
#define _PMBOOST_OTA_H_

#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <boost/filesystem.hpp>


using boost::uuids::detail::md5;

std::vector<std::string> split(const std::string& s, char delimiter);
std::string toString(const md5::digest_type& digest);

struct FileStat {
    bool newFile;
    std::string file;
    std::string md5;
    std::string hw;
    std::string role;
};

FileStat addFile(std::shared_ptr<std::map<std::string, std::string>> files,
                 boost::filesystem::path p, int dur);

#endif
