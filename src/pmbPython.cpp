#include "pmbPython.hpp"
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>


WiFiClass WiFi;
ESPClass ESP;

namespace po = boost::program_options;


painlessmesh::logger::LogClass Log;

// Will be used to obtain a seed for the random number engine
static std::random_device rd;
static std::mt19937 gen(rd());

uint32_t runif(uint32_t from, uint32_t to) {
  std::uniform_int_distribution<uint32_t> distribution(from, to);
  return distribution(gen);
}


PyMesh::PyMesh() : client(io_service) {
   nodeId = runif(0, std::numeric_limits<uint32_t>::max());
}

void PyMesh::pyInit(const std::string &ip, int port ) {
  Log.setLogLevel(ERROR);
  mesh.init(&scheduler, nodeId, port);
  painlessmesh::tcp::connect<MeshConnection, painlessMesh>(
                      (client), boost::asio::ip::address::from_string(ip), port, mesh);
}



template <class T>
// bool contains(T &v, T::value_type const value) {
bool contains(T& v, std::string const value) {
  return std::find(v.begin(), v.end(), value) != v.end();
}

std::string timeToString() {
  boost::posix_time::ptime timeLocal =
      boost::posix_time::second_clock::local_time();
  return to_iso_extended_string(timeLocal);
}


using namespace boost::python;


BOOST_PYTHON_MODULE(pmbPython_ext)
{
  using namespace boost::python;
  // def("greet", greet);
  
  class_<std::vector<std::string> >("StringList")
        .def(vector_indexing_suite<std::vector<std::string> >());
        
  class_<std::vector<uint32_t> >("uint32_tList")
        .def(vector_indexing_suite<std::vector<uint32_t> >());
        

  class_<PyMesh, boost::noncopyable>("PyMesh", init<>() )
    .def("pyInit", &PyMesh::pyInit)
    .def("poll", &PyMesh::poll)       
    .def("update", &PyMesh::update)
    .def("stop", &PyMesh::stop )   
    .def("getNodeId", &PyMesh::getNodeId ) 
    .def("sendSingle", &PyMesh::sendSingle )   
    .def("onReceived", &PyMesh::onReceived )
    .def("onNewConnection", &PyMesh::onNewConnection )
    .def("onDroppedConnection", &PyMesh::onDroppedConnection )
    .def("onChangedConnections", &PyMesh::onChangedConnections  )
    .def("onNodeTimeAdjusted", &PyMesh::onNodeTimeAdjusted  )
    .def("onNodeDelayReceived", &PyMesh::onNodeDelayReceived )
    .def("isConnected", &PyMesh::isConnected )
    .def("getNodeList", &PyMesh::getNodeList )
    .def("subConnectionJson", &PyMesh::subConnectionJson )
    ;

}
