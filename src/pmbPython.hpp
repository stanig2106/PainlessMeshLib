#ifndef _PMBPYTHON_HPP_
#define _PMBPYTHON_HPP_

#include <boost/python.hpp>

#include <algorithm>
#include <boost/algorithm/hex.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <iterator>

#include "Arduino.h"

#include "painlessMesh.h"
#include "painlessMeshConnection.h"
#include "plugin/performance.hpp"

#undef F
#include <boost/date_time.hpp>
#include <boost/program_options.hpp>
#define F(string_literal) string_literal

#include <iostream>
#include <iterator>
#include <limits>
#include <random>


class PyMesh {
     painlessMesh mesh;

    Scheduler scheduler;
    boost::asio::io_service io_service;
    AsyncClient client;
    boost::python::object receivedCallback;
    boost::python::object newConnectionCallback;

    size_t nodeId;
    PyMesh( const PyMesh & );

public:
  PyMesh() ;

    void pyInit(const std::string &ip, int port );
    void update() {
	mesh.update();
    }

    void poll() {
	io_service.poll();
    }
    
    void stop() {
	mesh.stop();
    }
    
    unsigned getNodeId() {
	return mesh.getNodeId();
    }
    
    void sendSingle(unsigned long  id, const std::string &msg) {
	mesh.sendSingle(id,msg);
    }
    
    bool startDelayMeas(uint32_t id) {
	return mesh.startDelayMeas(id);
    }
    
    void onReceived( PyObject *cb ) {
      auto pcb = borrow(cb);
      mesh.onReceive([pcb](uint32_t nodeId, std::string& msg) {
	  boost::python::call<void>(pcb.ptr(), nodeId, msg);
	  }
	);
    }

    void onNewConnection( PyObject *cb ) {
      auto pcb = borrow(cb);
      mesh.onNewConnection([pcb](uint32_t nodeId) {
	  boost::python::call<void>(pcb.ptr(), nodeId);
	  }
	);
      }
   
    void onDroppedConnection( PyObject *cb ) {
      auto pcb = borrow(cb);
      mesh.onDroppedConnection([pcb](uint32_t nodeId ) {
	  boost::python::call<void>(pcb.ptr(), nodeId );
	  }
	);
    }

    void onChangedConnections( PyObject *cb ) {
	auto pcb = borrow(cb);
	mesh.onChangedConnections([pcb]() {
	  boost::python::call<void>(pcb.ptr() );
	  }
	);
    }

    void onNodeTimeAdjusted( PyObject *cb ) {
	auto pcb = borrow(cb);
	mesh.onNodeTimeAdjusted([pcb](int offset) {
	  boost::python::call<void>(pcb.ptr(), offset );
	  }
	);
    }

    void onNodeDelayReceived( PyObject *cb ) {
	auto pcb = borrow(cb);
	mesh.onNodeDelayReceived([pcb](uint32_t nodeId, int32_t delay) {
	  boost::python::call<void>(pcb.ptr(), nodeId, delay );
	  }
	);
    }

    bool isConnected(uint32_t nodeId) {
	return mesh.isConnected( nodeId );
    }

    std::vector<uint32_t> getNodeList(bool includeSelf = false) {
	std::vector<uint32_t> rv;
	for( auto node : mesh.getNodeList( includeSelf ) ) {
	    rv.push_back(node);
	}
	return rv;
    }

    std::string subConnectionJson(bool pretty = false) {
	return mesh.subConnectionJson(pretty);
    }
	
private:
	boost::python::object borrow(PyObject *pyobj) {
      return boost::python::object(boost::python::handle<>(boost::python::borrowed(pyobj))); 
	}

};

#endif  //_PMBPYTHON_HPP_
