/*
 *      Recorder -- A package for Algorithmic Differentiation with CasADi
 *
 *      Copyright (C) 2019 The Authors
 *      Author: Joris Gillis
 *      Contributor: Antoine Falisse
 *
 *      Licensed under the Apache License, Version 2.0 (the "License"); you
 *      may not use this file except in compliance with the License. You may
 *      obtain a copy of the License at
 *      http://www.apache.org/licenses/LICENSE-2.0.
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *      implied. See the License for the specific language governing
 *      permissions and limitations under the License.
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <stdexcept>
#include <iomanip>
#include <cmath>
//#include "SimTKcommon/internal/Recorder.h"
#include "recorder.h"

static int counter_asserts = 0;

Recorder::Recorder(const Recorder& r) {
  value_ = r.value_;
  if (r.is_symbol()) {
    id_ = get_id();
    stream() << "    a" + std::to_string(id_) << " = " << r.repr() 
             << output_string("#", ";%") << " copy constructor" << value_ << std::endl;
  } else {
    id_ = -1;
  }
}

Recorder::~Recorder() {
       if(id_ == 0) std::cout << "goodbye" << id_ << ":" << value_ << std::endl;
}

Recorder::Recorder() : id_(-1), value_(3.14) {}
Recorder::Recorder(double value) : id_(-1), value_(value) {}

// select python or matlab output
// TODO: prevent switching while file is open
int Recorder::output_file_type = 0; // default .py
void Recorder::set_output_python() {
  output_file_type = 0;
}
void Recorder::set_output_matlab() {
  output_file_type = 1;
}

void Recorder::operator<<=(double value) {
  if (is_symbol()) throw std::runtime_error("Needs to be symbolic");
  id_ = get_id();
  switch (output_file_type) {
    case 0: // python
      stream() << "    if nom:" << std::endl;
      stream() << "        " << repr() << " = " << value << std::endl;
      stream() << "    else:" << std::endl;
      stream() << "        " << repr() << " = x[" << counter_input << "]" << std::endl;
      break;
    case 1: // matlab
      stream() << "    if nom" << std::endl;
      stream() << "        " << repr() << " = " << value << ";" << std::endl;
      stream() << "    else" << std::endl;
      stream() << "        " << repr() << " = x(" << counter_input+1 << ");" << std::endl;
      stream() << "    end" << std::endl;
      break;
  }
  counter_input++;
  value_ = value;
}

Recorder& Recorder::operator = ( const Recorder& r) {
  value_ = r.value_;
  if (r.is_symbol()) {
    id_ = get_id();
    stream() << "    a" + std::to_string(id_) << " = " << r.repr() 
             << output_string("#", ";%") << " copy assignment" << value_ << std::endl;
  } else {
    id_ = -1;
  }
  return *this;
}

void Recorder::operator>>=(double& value) {
  if (!is_symbol()) throw std::runtime_error("Needs to be symbolic");
  switch (output_file_type) {
    case 0: // python
      stream() << "    if not nom:" << std::endl;
      stream() << "        y.append(" << repr() << ")#" << value_  << std::endl;
    break;
    case 1: // matlab
      stream() << "    if ~nom" << std::endl;
      stream() << "      y{" << counter_output+1 << "} = " << repr() << ";%" << value_  << std::endl;
      stream() << "    end" << std::endl;
      break;
  }
  counter_output++;
  value = value_;
}

double Recorder::getValue() const {return value_;}

Recorder operator+(const Recorder& lhs, const Recorder& rhs) {
  return Recorder::from_binary(lhs, rhs, lhs.value_ + rhs.value_, Recorder::output_string("ca.plus", "plus"));
}
Recorder operator*(const Recorder& lhs, const Recorder& rhs) {
  return Recorder::from_binary(lhs, rhs, lhs.value_ * rhs.value_, Recorder::output_string("ca.times", "times"));
}
Recorder operator-(const Recorder& lhs, const Recorder& rhs) {
  return Recorder::from_binary(lhs, rhs, lhs.value_ - rhs.value_, Recorder::output_string("ca.minus", "minus"));
}
Recorder operator/(const Recorder& lhs, const Recorder& rhs) {
  return Recorder::from_binary(lhs, rhs, lhs.value_ / rhs.value_, Recorder::output_string("ca.rdivide", "rdivide"));
}
bool operator>=(const Recorder& lhs, const Recorder& rhs) {
  return static_cast<bool>(Recorder::from_binary(lhs, rhs, lhs.value_ >= rhs.value_, Recorder::output_string("ca.ge", "ge")));
}
bool operator<=(const Recorder& lhs, const Recorder& rhs) {
  return static_cast<bool>(Recorder::from_binary(lhs, rhs, lhs.value_ <= rhs.value_, Recorder::output_string("ca.le", "le")));
}
bool operator>(const Recorder& lhs, const Recorder& rhs) {
  return static_cast<bool>(Recorder::from_binary(lhs, rhs, lhs.value_ > rhs.value_, Recorder::output_string("ca.gt", "gt")));
}
bool operator<(const Recorder& lhs, const Recorder& rhs) {
  return static_cast<bool>(Recorder::from_binary(lhs, rhs, lhs.value_ < rhs.value_, Recorder::output_string("ca.lt", "lt")));
}
bool operator!=(const Recorder& lhs, const Recorder& rhs) {
  return static_cast<bool>(Recorder::from_binary(lhs, rhs, lhs.value_ != rhs.value_, Recorder::output_string("ca.ne", "ne")));
}
bool operator==(const Recorder& lhs, const Recorder& rhs) {
  return static_cast<bool>(Recorder::from_binary(lhs, rhs, lhs.value_ == rhs.value_, Recorder::output_string("ca.eq", "eq")));
}
Recorder operator-(const Recorder& arg) {
    return Recorder::from_unary(arg, -arg.value_, Recorder::output_string("-", "uminus"));
}
Recorder pow( const Recorder&lhs, const Recorder& rhs) {
    return Recorder::from_binary(lhs, rhs, pow(lhs.value_,rhs.value_), Recorder::output_string("ca.power","power"));
}
Recorder fmax ( const Recorder&lhs, const Recorder& rhs) {
	return Recorder::from_binary(lhs, rhs, fmax(lhs.value_,rhs.value_), Recorder::output_string("ca.fmax", "max"));
}
Recorder fmin ( const Recorder&lhs, const Recorder& rhs) {
	return Recorder::from_binary(lhs, rhs, fmin(lhs.value_,rhs.value_), Recorder::output_string("ca.fmin", "min"));
}
Recorder atan2 ( const Recorder&lhs, const Recorder& rhs) {
	return Recorder::from_binary(lhs, rhs, atan2(lhs.value_,rhs.value_), Recorder::output_string("ca.atan2", "atan2"));
}
Recorder exp(const Recorder& arg) {
    return Recorder::from_unary(arg, exp(arg.value_), Recorder::output_string("ca.exp", "exp"));
}
Recorder log(const Recorder& arg) {
    return Recorder::from_unary(arg, log(arg.value_), Recorder::output_string("ca.log", "log"));
}
Recorder sqrt(const Recorder& arg) {
    return Recorder::from_unary(arg, sqrt(arg.value_), Recorder::output_string("ca.sqrt", "sqrt"));
}
Recorder sin(const Recorder& arg) {
    return Recorder::from_unary(arg, sin(arg.value_), Recorder::output_string("ca.sin", "sin"));
}
Recorder cos(const Recorder& arg) {
    return Recorder::from_unary(arg, cos(arg.value_), Recorder::output_string("ca.cos", "cos"));
}
Recorder tan(const Recorder& arg) {
    return Recorder::from_unary(arg, tan(arg.value_), Recorder::output_string("ca.tan", "tan"));
}
Recorder asin(const Recorder& arg) {
    return Recorder::from_unary(arg, asin(arg.value_), Recorder::output_string("ca.asin", "asin"));
}
Recorder acos(const Recorder& arg) {
    return Recorder::from_unary(arg, acos(arg.value_), Recorder::output_string("ca.acos", "acos"));
}
Recorder atan(const Recorder& arg) {
    return Recorder::from_unary(arg, atan(arg.value_), Recorder::output_string("ca.atan", "atan"));
}
Recorder log10(const Recorder& arg) {
    return Recorder::from_unary(arg, log10(arg.value_), Recorder::output_string("ca.log10", "log10"));
}
Recorder sinh(const Recorder& arg) {
    return Recorder::from_unary(arg, sinh(arg.value_), Recorder::output_string("ca.sinh", "sinh"));
}
Recorder cosh(const Recorder& arg) {
    return Recorder::from_unary(arg, cosh(arg.value_), Recorder::output_string("ca.cosh", "cosh"));
}
Recorder tanh(const Recorder& arg) {
    return Recorder::from_unary(arg, tanh(arg.value_), Recorder::output_string("ca.tanh", "tanh"));
}
Recorder asinh(const Recorder& arg) {
    return Recorder::from_unary(arg, asinh(arg.value_), Recorder::output_string("ca.asinh", "asinh"));
}
Recorder acosh(const Recorder& arg) {
    return Recorder::from_unary(arg, acosh(arg.value_), Recorder::output_string("ca.acosh", "acosh"));
}
Recorder atanh(const Recorder& arg) {
    return Recorder::from_unary(arg, atanh(arg.value_), Recorder::output_string("ca.atanh", "atanh"));
}
Recorder erf(const Recorder& arg) {
    return Recorder::from_unary(arg, erf(arg.value_), Recorder::output_string("ca.erf", "erf"));
}
Recorder fabs(const Recorder& arg) {
    return Recorder::from_unary(arg, fabs(arg.value_), Recorder::output_string("ca.fabs", "abs"));
}
Recorder ceil(const Recorder& arg) {
    return Recorder::from_unary(arg, ceil(arg.value_), Recorder::output_string("ca.ceil", "ceil"));
}
Recorder floor(const Recorder& arg) {
    return Recorder::from_unary(arg, floor(arg.value_), Recorder::output_string("ca.floor", "floor"));
}

Recorder::operator bool() const {
  bool ret = value_==1;

  if (is_symbol()) {
    counter_bool++;
    switch (output_file_type) {
      case 0: // python
        stream() << "    a.append(" << repr() << "-" << value_ << ")#" << value_  << std::endl;
        stream() << "    b.append(" << repr() << ")#" << value_  << std::endl;
      break;
      case 1: // matlab
        stream() << "    a{" << counter_asserts+1 << "} = " << repr() << "-" << value_ << ";%" << value_  << std::endl;
        stream() << "    b{" << counter_asserts+1 << "} = " << repr() << ";%" << value_ << std::endl;
        break;
    }
    
  }
  return ret;
}

std::ostream& operator<<(std::ostream &stream, const Recorder& obj) {
  obj.disp(stream);
  return stream;
}

std::istream& operator >> (std::istream& is, const Recorder& a) {
  throw std::runtime_error("No way!");
}

void Recorder::stop_recording() {
  switch (output_file_type) {
    case 0: // python
      stream() << "    if not nom:" << std::endl;
      stream() << "        y = ca.vertcat(*y)" << std::endl;
      if (counter_bool > 0) {
        stream() << "    a = ca.vertcat(*a)" << std::endl;
        stream() << "    b = ca.vertcat(*b)" << std::endl;
      }
      stream() << "    return y, a, b" << std::endl; 
    break;
    case 1: // matlab
      stream() << "    if ~nom, y = vertcat(y{:}); end" << std::endl;
      if (counter_bool > 0) {
        stream() << "    a = vertcat(a{:});" << std::endl;
        stream() << "    b = vertcat(b{:});" << std::endl;
      } else {
        stream() << "    a = {};" << std::endl;
        stream() << "    b = {};" << std::endl;
      }
      stream() << "end" << std::endl;
      break;
  }
  
  // stop wrting to file
  stream().close();
  stream_wrapper_.reset();

  // reset counters
  counter = 0;
  counter_input = 0;
  counter_output = 0;
  counter_bool = 0;
}

void Recorder::disp(std::ostream &stream) const {
  if (is_symbol()) {
    stream << "    [#" << id_ << "|" << value_ << "]";
  } else {
    stream << "    (" << value_ << ")";
  }
}

int Recorder::get_id() {
  counter++;
  return counter;
}

bool Recorder::is_symbol() const {
  return id_>=0;
}

std::string Recorder::repr() const {
  if (is_symbol()) {
    return "a" + std::to_string(id_);
  } else {
    std::stringstream ss;
    ss << std::scientific << std::setprecision(16);
    ss << value_;
    return ss.str();
  }
}

bool is_suspicious(double v) {
  return (v>0 && v <1e-200) || (v<0 && v >-1e-200);
}

Recorder Recorder::from_binary(const Recorder& lhs, const Recorder& rhs, double res, const std::string& op) {
  if (lhs.is_symbol() || rhs.is_symbol()) {
    int id = get_id();
    switch (output_file_type) {
      case 0: // python
        stream() << "    a" << id << " = " << op << "(" << lhs.repr() <<  "," <<   rhs.repr() << ")" << std::endl;
        stream() << "    if nom:" << std::endl;
        stream() << "        assert(" << "a" << id << "==" << res << ")" << std::endl;
        break;
      case 1: // matlab
        stream() << "    a" << id << " = " << op << "(" << lhs.repr() <<  "," <<   rhs.repr() << ");" << std::endl;
        stream() << "    if nom, assert(" << "a" << id << "==" << res << "); end" << std::endl;
        break;
    }
    
    if (is_suspicious(res)) {
      stream() << "    " << output_string("#", "%") << " suspicious activity" << std::endl;
    }

    return Recorder(res, id);
  } else {
    return Recorder(res);
  }
}

Recorder Recorder::from_unary(const Recorder& arg, double res, const std::string& op) {
  if (arg.is_symbol()) {
    int id = get_id();
    switch (output_file_type) {
      case 0: // python
        stream() << "    a" << id << " = " << op << "(" << arg.repr() << ")" << std::endl;
        stream() << "    if nom:" << std::endl;
        stream() << "        assert(" << "a" << id << "==" << res << ")" << std::endl;
        break;
      case 1: // matlab
        stream() << "    a" << id << " = " << op << "(" << arg.repr() << ");" << std::endl;
        stream() << "    if nom, assert(" << "a" << id << "==" << res << "); end" << std::endl;
        break;
    }
    if (is_suspicious(res)) {
      stream() << "    " << output_string("#", "%") << " suspicious activity" << std::endl;
    }

    return Recorder(res, id);
  } else {
    return Recorder(res);
  }
}

std::unique_ptr<StreamWrapper> Recorder::stream_wrapper_;

StreamWrapper::StreamWrapper(const std::string& filename, const int file_type) {
  // clean up filename and create filepath
  std::string filepath;
  switch (file_type) {
    case 0: // python
      filepath = std::filesystem::path(filename).replace_extension(".py").string();
      break;
    case 1: // matlab
      filepath = std::filesystem::path(filename).replace_extension(".m").string();
      break;
  }
  
  stream_ = std::ofstream(filepath);
  if (!stream_.is_open()) {
    throw std::runtime_error("Failed to open file: " + filepath);
  }
  stream_ << std::scientific << std::setprecision(16);

  switch (file_type) {
    case 0: // python	
      stream_ << "def " 
              << std::filesystem::path(filename).stem().string() 
              << "(*args):" << std::endl;
      stream_ << "    import casadi as ca" << std::endl;
      stream_ << "    nom = len(args) == 0" << std::endl;
      stream_ << "    if not nom:" << std::endl;
      stream_ << "        x = args[0]" << std::endl;
      stream_ << "    a = []" << std::endl;
      stream_ << "    b = []" << std::endl;
      stream_ << "    y = []" << std::endl;
      break;
    case 1: // matlab
      stream_ << "function [y,a,b]=" 
              << std::filesystem::path(filename).stem().string() 
              << "(x)" << std::endl;
      stream_ << "    nom = nargin==0;" << std::endl;
      break;
    }
}

std::ofstream& StreamWrapper::stream() {
  return stream_;
}

void Recorder::start_recording(const std::string& filename) {
  if (stream_wrapper_) {
    throw std::runtime_error("Recording already started.");
  }

  if (std::filesystem::path(filename).extension().string() == ".py") set_output_python();
  if (std::filesystem::path(filename).extension().string() == ".m") set_output_matlab();

  stream_wrapper_ = std::make_unique<StreamWrapper>(filename, output_file_type);
}

void Recorder::start_recording() {
  Recorder::start_recording("foo");
};

std::ofstream& Recorder::stream() {
  if (!stream_wrapper_) {
    Recorder::start_recording();
  }
  return stream_wrapper_->stream();
}

Recorder::Recorder(double value, int id) {
  id_ = id;
  value_ = value;
}

int Recorder::counter = 0;
int Recorder::counter_input = 0;
int Recorder::counter_output = 0;
int Recorder::counter_bool = 0;
