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

//-------------------------------------------------------------------------
// python 
//-------------------------------------------------------------------------
PythonRecorder::PythonRecorder(const PythonRecorder& r) {
  value_ = r.value_;
  if (r.is_symbol()) {
    id_ = get_id();
    stream() << "    a" + std::to_string(id_) << " = " << r.repr() << "# copy constructor" << value_ << std::endl;
  } else {
    id_ = -1;
  }
}

PythonRecorder::~PythonRecorder() {
  if(id_ == 0) std::cout << "goodbye" << id_ << ":" << value_ << std::endl;
}

PythonRecorder::PythonRecorder() : id_(-1), value_(3.14) {}
PythonRecorder::PythonRecorder(double value) : id_(-1), value_(value) {}

void PythonRecorder::operator<<=(double value) {
  if (is_symbol()) throw std::runtime_error("Needs to be symbolic");
  id_ = get_id();
  stream() << "    if nom:" << std::endl;
  stream() << "        " << repr() << " = " << value << std::endl;
  stream() << "    else:" << std::endl;
  stream() << "        " << repr() << " = x[" << counter_input << "]" << std::endl;
  counter_input++;
  value_ = value;
}

PythonRecorder& PythonRecorder::operator = ( const PythonRecorder& r) {
  value_ = r.value_;
  if (r.is_symbol()) {
    id_ = get_id();
    stream() << "    a" + std::to_string(id_) << " = " << r.repr() << "# copy assignment" << value_ << std::endl;
  } else {
    id_ = -1;
  }
  return *this;
}

void PythonRecorder::operator>>=(double& value) {
  if (!is_symbol()) throw std::runtime_error("Needs to be symbolic");
  stream() << "    if not nom:" << std::endl;
  stream() << "        y.append(" << repr() << ")#" << value_  << std::endl;
  counter_output++;
  value = value_;
}

double PythonRecorder::getValue() const {return value_;}

PythonRecorder operator+(const PythonRecorder& lhs, const PythonRecorder& rhs) {
  return PythonRecorder::from_binary(lhs, rhs, lhs.value_ + rhs.value_, "ca.plus");
}
PythonRecorder operator*(const PythonRecorder& lhs, const PythonRecorder& rhs) {
  return PythonRecorder::from_binary(lhs, rhs, lhs.value_ * rhs.value_, "ca.times");
}
PythonRecorder operator-(const PythonRecorder& lhs, const PythonRecorder& rhs) {
  return PythonRecorder::from_binary(lhs, rhs, lhs.value_ - rhs.value_, "ca.minus");
}
PythonRecorder operator/(const PythonRecorder& lhs, const PythonRecorder& rhs) {
  return PythonRecorder::from_binary(lhs, rhs, lhs.value_ / rhs.value_, "ca.rdivide");
}
bool operator>=(const PythonRecorder& lhs, const PythonRecorder& rhs) {
  return static_cast<bool>(PythonRecorder::from_binary(lhs, rhs, lhs.value_ >= rhs.value_, "ca.ge"));
}
bool operator<=(const PythonRecorder& lhs, const PythonRecorder& rhs) {
  return static_cast<bool>(PythonRecorder::from_binary(lhs, rhs, lhs.value_ <= rhs.value_, "ca.le"));
}
bool operator>(const PythonRecorder& lhs, const PythonRecorder& rhs) {
  return static_cast<bool>(PythonRecorder::from_binary(lhs, rhs, lhs.value_ > rhs.value_, "ca.gt"));
}
bool operator<(const PythonRecorder& lhs, const PythonRecorder& rhs) {
  return static_cast<bool>(PythonRecorder::from_binary(lhs, rhs, lhs.value_ < rhs.value_, "ca.lt"));
}
bool operator!=(const PythonRecorder& lhs, const PythonRecorder& rhs) {
  return static_cast<bool>(PythonRecorder::from_binary(lhs, rhs, lhs.value_ != rhs.value_, "ca.ne"));
}
bool operator==(const PythonRecorder& lhs, const PythonRecorder& rhs) {
  return static_cast<bool>(PythonRecorder::from_binary(lhs, rhs, lhs.value_ == rhs.value_, "ca.eq"));
}
PythonRecorder operator-(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, -arg.value_, "-");
}
PythonRecorder pow( const PythonRecorder&lhs, const PythonRecorder& rhs) {
    return PythonRecorder::from_binary(lhs, rhs, pow(lhs.value_,rhs.value_), "ca.power");
}
PythonRecorder fmax ( const PythonRecorder&lhs, const PythonRecorder& rhs) {
	return PythonRecorder::from_binary(lhs, rhs, fmax(lhs.value_,rhs.value_), "ca.fmax");
}
PythonRecorder fmin ( const PythonRecorder&lhs, const PythonRecorder& rhs) {
	return PythonRecorder::from_binary(lhs, rhs, fmin(lhs.value_,rhs.value_), "ca.fmin");
}
PythonRecorder atan2 ( const PythonRecorder&lhs, const PythonRecorder& rhs) {
	return PythonRecorder::from_binary(lhs, rhs, atan2(lhs.value_,rhs.value_), "ca.atan2");
}
PythonRecorder exp(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, exp(arg.value_), "ca.exp");
}
PythonRecorder log(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, log(arg.value_), "ca.log");
}
PythonRecorder sqrt(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, sqrt(arg.value_), "ca.sqrt");
}
PythonRecorder sin(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, sin(arg.value_), "ca.sin");
}
PythonRecorder cos(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, cos(arg.value_), "ca.cos");
}
PythonRecorder tan(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, tan(arg.value_), "ca.tan");
}
PythonRecorder asin(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, asin(arg.value_), "ca.asin");
}
PythonRecorder acos(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, acos(arg.value_), "ca.acos");
}
PythonRecorder atan(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, atan(arg.value_), "ca.atan");
}
PythonRecorder log10(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, log10(arg.value_), "ca.log10");
}
PythonRecorder sinh(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, sinh(arg.value_), "ca.sinh");
}
PythonRecorder cosh(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, cosh(arg.value_), "ca.cosh");
}
PythonRecorder tanh(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, tanh(arg.value_), "ca.tanh");
}
PythonRecorder asinh(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, asinh(arg.value_), "ca.asinh");
}
PythonRecorder acosh(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, acosh(arg.value_), "ca.acosh");
}
PythonRecorder atanh(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, atanh(arg.value_), "ca.atanh");
}
PythonRecorder erf(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, erf(arg.value_), "ca.erf");
}
PythonRecorder fabs(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, fabs(arg.value_), "ca.fabs");
}
PythonRecorder ceil(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, ceil(arg.value_), "ca.ceil");
}
PythonRecorder floor(const PythonRecorder& arg) {
    return PythonRecorder::from_unary(arg, floor(arg.value_), "ca.floor");
}

PythonRecorder::operator bool() const {
   bool ret = value_==1;

    if (is_symbol()) {
      counter_bool++;
      stream() << "    a.append(" << repr() << "-" << value_ << ")#" << value_  << std::endl;
      stream() << "    b.append(" << repr() << ")#" << value_  << std::endl;
    }
    return ret;
}

std::ostream& operator<<(std::ostream &stream, const PythonRecorder& obj) {
  obj.disp(stream);
  return stream;
}

std::istream& operator >> (std::istream& is, const PythonRecorder& a) {
  throw std::runtime_error("No way!");
}

void PythonRecorder::stop_recording() {
  stream() << "    if not nom:" << std::endl;
  stream() << "        y = ca.vertcat(*y)" << std::endl;
  
  if (counter_bool > 0) {
    stream() << "    a = ca.vertcat(*a)" << std::endl;
    stream() << "    b = ca.vertcat(*b)" << std::endl;
  }

  stream() << "    return y, a, b" << std::endl;

  python_stream_wrapper_.reset();
}

void PythonRecorder::disp(std::ostream &stream) const {
  if (is_symbol()) {
    stream << "    [#" << id_ << "|" << value_ << "]";
  } else {
    stream << "    (" << value_ << ")";
  }
}

int PythonRecorder::get_id() {
  counter++;
  return counter;
}

bool PythonRecorder::is_symbol() const {
  return id_>=0;
}

std::string PythonRecorder::repr() const {
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

PythonRecorder PythonRecorder::from_binary(const PythonRecorder& lhs, const PythonRecorder& rhs, double res, const std::string& op) {
  if (lhs.is_symbol() || rhs.is_symbol()) {
    int id = get_id();
    stream() << "    a" << id << " = " << op << "(" << lhs.repr() <<  "," <<   rhs.repr() << ")" << std::endl;
    stream() << "    if nom:" << std::endl;
	  stream() << "        assert(" << "a" << id << "==" << res << ")" << std::endl;

    if (is_suspicious(res)) {
      stream() << "    # suspicious activity" << std::endl;
    }

    return PythonRecorder(res, id);
  } else {
    return PythonRecorder(res);
  }
}

PythonRecorder PythonRecorder::from_unary(const PythonRecorder& arg, double res, const std::string& op) {
  if (arg.is_symbol()) {
    int id = get_id();
    stream() << "    a" << id << " = " << op << "(" << arg.repr() << ")" << std::endl;
    stream() << "    if nom:" << std::endl;
	  stream() << "        assert(" << "a" << id << "==" << res << ")" << std::endl;

    if (is_suspicious(res)) {
      stream() << "    # suspicious activity" << std::endl;
    }

    return PythonRecorder(res, id);
  } else {
    return PythonRecorder(res);
  }
}

// new PythonStreamWrapper
std::unique_ptr<PythonStreamWrapper> PythonRecorder::python_stream_wrapper_;

PythonStreamWrapper::PythonStreamWrapper(const std::string& filename)
    : stream_(std::filesystem::path(filename).replace_extension(".py").string(),
     std::ios::out) {
  if (!stream_.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }
  stream_ << std::scientific << std::setprecision(16);		
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
}

std::ofstream& PythonStreamWrapper::stream() {
  return stream_;
}

void PythonRecorder::start_recording(const std::string& filename) {
  if (python_stream_wrapper_) {
    throw std::runtime_error("Recording already started.");
  }
  python_stream_wrapper_ = std::make_unique<PythonStreamWrapper>(filename);
}

void PythonRecorder::start_recording() {
  PythonRecorder::start_recording("foo.py");
};

std::ofstream& PythonRecorder::stream() {
  if (!python_stream_wrapper_) {
    //throw std::runtime_error("Recording not started. Call start_recording() first.");
    PythonRecorder::start_recording();
  }
  return python_stream_wrapper_->stream();
}

PythonRecorder::PythonRecorder(double value, int id) {
  id_ = id;
  value_ = value;
}

int PythonRecorder::counter = 0;
int PythonRecorder::counter_input = 0;
int PythonRecorder::counter_output = 0;
int PythonRecorder::counter_bool = 0;

//-------------------------------------------------------------------------
// matlab 
//-------------------------------------------------------------------------

MatlabRecorder::MatlabRecorder(const MatlabRecorder& r) {
  value_ = r.value_;
  if (r.is_symbol()) {
    id_ = get_id();
    stream() << "a" + std::to_string(id_) << " = " << r.repr() << ";% copy constructor" << value_ << std::endl;
  } else {
    id_ = -1;
  }
}

MatlabRecorder::~MatlabRecorder() {
       if(id_ == 0) std::cout << "goodbye" << id_ << ":" << value_ << std::endl;
}

MatlabRecorder::MatlabRecorder() : id_(-1), value_(3.14) {}
MatlabRecorder::MatlabRecorder(double value) : id_(-1), value_(value) {}

void MatlabRecorder::operator<<=(double value) {
  if (is_symbol()) throw std::runtime_error("Needs to be symbolic");
  id_ = get_id();
  stream() << "if nom" << std::endl;
  stream() << "  " << repr() << " = " << value << ";" << std::endl;
  stream() << "else" << std::endl;
  stream() << "  " << repr() << " = x(" << counter_input+1 << ");" << std::endl;
  stream() << "end" << std::endl;
  counter_input++;
  value_ = value;
}

MatlabRecorder& MatlabRecorder::operator = ( const MatlabRecorder& r) {
  value_ = r.value_;
  if (r.is_symbol()) {
    id_ = get_id();
    stream() << "a" + std::to_string(id_) << " = " << r.repr() << ";% copy assignment" << value_ << std::endl;
  } else {
    id_ = -1;
  }
  return *this;
}

void MatlabRecorder::operator>>=(double& value) {
  if (!is_symbol()) throw std::runtime_error("Needs to be symbolic");
  stream() << "if ~nom" << std::endl;
  stream() << "  y{" << counter_output+1 << "} = " << repr() << ";%" << value_  << std::endl;
  stream() << "end" << std::endl;
  counter_output++;
  value = value_;
}

double MatlabRecorder::getValue() const {return value_;}

MatlabRecorder operator+(const MatlabRecorder& lhs, const MatlabRecorder& rhs) {
  return MatlabRecorder::from_binary(lhs, rhs, lhs.value_ + rhs.value_, "plus");
}
MatlabRecorder operator*(const MatlabRecorder& lhs, const MatlabRecorder& rhs) {
  return MatlabRecorder::from_binary(lhs, rhs, lhs.value_ * rhs.value_, "times");
}
MatlabRecorder operator-(const MatlabRecorder& lhs, const MatlabRecorder& rhs) {
  return MatlabRecorder::from_binary(lhs, rhs, lhs.value_ - rhs.value_, "minus");
}
MatlabRecorder operator/(const MatlabRecorder& lhs, const MatlabRecorder& rhs) {
  return MatlabRecorder::from_binary(lhs, rhs, lhs.value_ / rhs.value_, "rdivide");
}
bool operator>=(const MatlabRecorder& lhs, const MatlabRecorder& rhs) {
  return static_cast<bool>(MatlabRecorder::from_binary(lhs, rhs, lhs.value_ >= rhs.value_, "ge"));
}
bool operator<=(const MatlabRecorder& lhs, const MatlabRecorder& rhs) {
  return static_cast<bool>(MatlabRecorder::from_binary(lhs, rhs, lhs.value_ <= rhs.value_, "le"));
}
bool operator>(const MatlabRecorder& lhs, const MatlabRecorder& rhs) {
  return static_cast<bool>(MatlabRecorder::from_binary(lhs, rhs, lhs.value_ > rhs.value_, "gt"));
}
bool operator<(const MatlabRecorder& lhs, const MatlabRecorder& rhs) {
  return static_cast<bool>(MatlabRecorder::from_binary(lhs, rhs, lhs.value_ < rhs.value_, "lt"));
}
bool operator!=(const MatlabRecorder& lhs, const MatlabRecorder& rhs) {
  return static_cast<bool>(MatlabRecorder::from_binary(lhs, rhs, lhs.value_ != rhs.value_, "ne"));
}
bool operator==(const MatlabRecorder& lhs, const MatlabRecorder& rhs) {
  return static_cast<bool>(MatlabRecorder::from_binary(lhs, rhs, lhs.value_ == rhs.value_, "eq"));
}
MatlabRecorder operator-(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, -arg.value_, "uminus");
}
MatlabRecorder pow( const MatlabRecorder&lhs, const MatlabRecorder& rhs) {
    return MatlabRecorder::from_binary(lhs, rhs, pow(lhs.value_,rhs.value_), "power");
}
MatlabRecorder fmax ( const MatlabRecorder&lhs, const MatlabRecorder& rhs) {
	return MatlabRecorder::from_binary(lhs, rhs, fmax(lhs.value_,rhs.value_), "max");
}
MatlabRecorder fmin ( const MatlabRecorder&lhs, const MatlabRecorder& rhs) {
	return MatlabRecorder::from_binary(lhs, rhs, fmin(lhs.value_,rhs.value_), "min");
}
MatlabRecorder atan2 ( const MatlabRecorder&lhs, const MatlabRecorder& rhs) {
	return MatlabRecorder::from_binary(lhs, rhs, atan2(lhs.value_,rhs.value_), "atan2");
}
MatlabRecorder exp(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, exp(arg.value_), "exp");
}
MatlabRecorder log(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, log(arg.value_), "log");
}
MatlabRecorder sqrt(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, sqrt(arg.value_), "sqrt");
}
MatlabRecorder sin(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, sin(arg.value_), "sin");
}
MatlabRecorder cos(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, cos(arg.value_), "cos");
}
MatlabRecorder tan(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, tan(arg.value_), "tan");
}
MatlabRecorder asin(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, asin(arg.value_), "asin");
}
MatlabRecorder acos(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, acos(arg.value_), "acos");
}
MatlabRecorder atan(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, atan(arg.value_), "atan");
}
MatlabRecorder log10(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, log10(arg.value_), "log10");
}
MatlabRecorder sinh(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, sinh(arg.value_), "sinh");
}
MatlabRecorder cosh(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, cosh(arg.value_), "cosh");
}
MatlabRecorder tanh(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, tanh(arg.value_), "tanh");
}
MatlabRecorder asinh(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, asinh(arg.value_), "asinh");
}
MatlabRecorder acosh(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, acosh(arg.value_), "acosh");
}
MatlabRecorder atanh(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, atanh(arg.value_), "atanh");
}
MatlabRecorder erf(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, erf(arg.value_), "erf");
}
MatlabRecorder fabs(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, fabs(arg.value_), "abs");
}
MatlabRecorder ceil(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, ceil(arg.value_), "ceil");
}
MatlabRecorder floor(const MatlabRecorder& arg) {
    return MatlabRecorder::from_unary(arg, floor(arg.value_), "floor");
}

MatlabRecorder::operator bool() const {
   bool ret = value_==1;

    if (is_symbol()) {
      counter_bool++;
      stream() << "a{" << counter_asserts+1 << "} = " << repr() << "-" << value_ << ";%" << value_  << std::endl;
      stream() << "b{" << counter_asserts + 1 << "} = " << repr() << ";%" << value_ << std::endl;
    }
    return ret;
}

std::ostream& operator<<(std::ostream &stream, const MatlabRecorder& obj) {
  obj.disp(stream);
  return stream;
}

std::istream& operator >> (std::istream& is, const MatlabRecorder& a) {
  throw std::runtime_error("No way!");
}

void MatlabRecorder::stop_recording() {
  stream() << "if ~nom, y = vertcat(y{:}); end" << std::endl;
  if (counter_bool > 0) {
    stream() << "a = vertcat(a{:});" << std::endl;
    stream() << "b = vertcat(b{:});" << std::endl;
  } else {
    stream() << "a = {};" << std::endl;
    stream() << "b = {};" << std::endl;
  }
  stream() << "end" << std::endl;

  matlab_stream_wrapper_.reset();
}

void MatlabRecorder::disp(std::ostream &stream) const {
  if (is_symbol()) {
    stream << "[#" << id_ << "|" << value_ << "]";
  } else {
    stream << "(" << value_ << ")";
  }
}

int MatlabRecorder::get_id() {
  counter++;
  return counter;
}

bool MatlabRecorder::is_symbol() const {
  return id_>=0;
}

std::string MatlabRecorder::repr() const {
  if (is_symbol()) {
    return "a" + std::to_string(id_);
  } else {
    std::stringstream ss;
    ss << std::scientific << std::setprecision(16);
    ss << value_;
    return ss.str();
  }
}

MatlabRecorder MatlabRecorder::from_binary(const MatlabRecorder& lhs, const MatlabRecorder& rhs, double res, const std::string& op) {
  if (lhs.is_symbol() || rhs.is_symbol()) {
    int id = get_id();
    stream() << "a" << id << " = " << op << "(" << lhs.repr() <<  "," <<   rhs.repr() << ");" << std::endl;
    stream() << "if nom, assert(" << "a" << id << "==" << res << "); end" << std::endl;

    if (is_suspicious(res)) {
      stream() << "% suspicious activity" << std::endl;
    }

    return MatlabRecorder(res, id);
  } else {
    return MatlabRecorder(res);
  }
}

MatlabRecorder MatlabRecorder::from_unary(const MatlabRecorder& arg, double res, const std::string& op) {
  if (arg.is_symbol()) {
    int id = get_id();
    stream() << "a" << id << " = " << op << "(" << arg.repr() << ");" << std::endl;
    stream() << "if nom, assert(" << "a" << id << "==" << res << "); end" << std::endl;

    if (is_suspicious(res)) {
      stream() << "% suspicious activity" << std::endl;
    }

    return MatlabRecorder(res, id);
  } else {
    return MatlabRecorder(res);
  }
}

std::unique_ptr<MatlabStreamWrapper> MatlabRecorder::matlab_stream_wrapper_;

MatlabStreamWrapper::MatlabStreamWrapper(const std::string& filename)
    : stream_(std::filesystem::path(filename).replace_extension(".m").string(),
     std::ios::out) {
  if (!stream_.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }
  stream_ << std::scientific << std::setprecision(16);
  stream_ << "function [y,a,b]=" 
          << std::filesystem::path(filename).stem().string() 
          <<"(x)" << std::endl;
  stream_ << "nom = nargin==0;" << std::endl;
};

std::ofstream& MatlabStreamWrapper::stream() {
  return stream_;
}

void MatlabRecorder::start_recording(const std::string& filename) {
  if (matlab_stream_wrapper_) {
    throw std::runtime_error("Recording already started.");
  }
  matlab_stream_wrapper_ = std::make_unique<MatlabStreamWrapper>(filename);
}

void MatlabRecorder::start_recording() {
  MatlabRecorder::start_recording("foo.mat");
};

std::ofstream& MatlabRecorder::stream() {
  if (!matlab_stream_wrapper_) {
    //throw std::runtime_error("Recording not started. Call start_recording() first.");
    MatlabRecorder::start_recording();
  }
  return matlab_stream_wrapper_->stream();
}

MatlabRecorder::MatlabRecorder(double value, int id) {
  id_ = id;
  value_ = value;
}

int MatlabRecorder::counter = 0;
int MatlabRecorder::counter_input = 0;
int MatlabRecorder::counter_output = 0;
int MatlabRecorder::counter_bool = 0;
