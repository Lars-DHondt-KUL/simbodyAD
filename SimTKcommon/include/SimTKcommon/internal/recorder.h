#ifndef Recorder_H_
#define Recorder_H_

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
#include <memory>
#include <string>

#if defined _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif // defined _WIN32

class PythonStreamWrapper {
public:
  PythonStreamWrapper(const std::string& filename);
  std::ofstream& stream();

private:
  std::ofstream stream_;
};

class MatlabStreamWrapper {
public:
  MatlabStreamWrapper(const std::string& filename);
  std::ofstream& stream();

private:
  std::ofstream stream_;
};

/* TODO (?): 
  Recorder class that is cast to PythonRecorder for compatibility

class DLL_EXPORT Recorder {
public:
  ~Recorder();
  Recorder();
  Recorder(double value);
  static void stop_recording();
  static void start_recording();
  static void start_recording(const std::string& filename);

protected:
  void disp(std::ostream &stream) const;
  static int get_id();
  bool is_symbol() const;
  std::string repr() const;
  static std::ofstream& stream();
  double value_;
  int id_;

  static int counter;
  static int counter_input;
  static int counter_output;
  static int counter_bool;

};
*/

class DLL_EXPORT PythonRecorder {
public:
  ~PythonRecorder();
  PythonRecorder();
  PythonRecorder(double value);
  void operator<<=(double value);
  void operator>>=(double& value);
  explicit operator bool() const;
  PythonRecorder(const PythonRecorder& r);
  friend DLL_EXPORT std::ostream& operator<<(std::ostream &stream, const PythonRecorder& obj);
  static void stop_recording();
  static void start_recording();
  static void start_recording(const std::string& filename);

  /* Assignments */
  double getValue() const;
  inline double value() const {return getValue();}
  inline PythonRecorder& operator = ( double arg) { return operator=(PythonRecorder(arg)); }
  PythonRecorder& operator = ( const PythonRecorder& );

  /* IO friends */
  friend DLL_EXPORT std::istream& operator >> (std::istream& is, const PythonRecorder& a);

  /* Operation and assignment */
  inline PythonRecorder& operator += ( double value ) { return operator+=(PythonRecorder(value)); }
  inline PythonRecorder& operator += ( const PythonRecorder& value) { return operator=(*this+value); }
  inline PythonRecorder& operator -= ( double value ) { return operator-=(PythonRecorder(value)); }
  inline PythonRecorder& operator -= ( const PythonRecorder& value) { return operator=(*this-value); }
  inline PythonRecorder& operator *= ( double value)  { return operator*=(PythonRecorder(value)); }
  inline PythonRecorder& operator *= ( const PythonRecorder& value) { return operator=(*this*value); }
  inline PythonRecorder& operator /= ( double value)  { return operator/=(PythonRecorder(value)); }
  inline PythonRecorder& operator /= ( const PythonRecorder& value) { return operator=(*this/value); }

  /* Comparison */
  friend bool DLL_EXPORT operator != ( const PythonRecorder&, const PythonRecorder& );
  friend bool DLL_EXPORT operator == ( const PythonRecorder&, const PythonRecorder& );
  friend bool DLL_EXPORT operator <= ( const PythonRecorder&, const PythonRecorder& );
  friend bool DLL_EXPORT operator >= ( const PythonRecorder&, const PythonRecorder& );
  friend bool DLL_EXPORT operator >  ( const PythonRecorder&, const PythonRecorder& );
  friend bool DLL_EXPORT operator <  ( const PythonRecorder&, const PythonRecorder& );
  inline friend bool operator != (double lhs, const PythonRecorder& rhs) { return PythonRecorder(lhs)!=rhs; }
  inline friend bool operator == ( double lhs, const PythonRecorder& rhs) { return PythonRecorder(lhs)==rhs; }
  inline friend bool operator <= ( double lhs, const PythonRecorder& rhs) { return PythonRecorder(lhs)<=rhs; }
  inline friend bool operator >= ( double lhs, const PythonRecorder& rhs) { return PythonRecorder(lhs)>=rhs; }
  inline friend bool operator >  ( double lhs, const PythonRecorder& rhs) { return PythonRecorder(lhs)>rhs; }
  inline friend bool operator <  ( double lhs, const PythonRecorder& rhs) { return PythonRecorder(lhs)<rhs; }

  /* Sign operators */
  inline friend PythonRecorder operator + ( const PythonRecorder& x ) { return x; }
  friend PythonRecorder DLL_EXPORT  operator - ( const PythonRecorder& x );

  /* Binary operators */
  friend PythonRecorder DLL_EXPORT operator + ( const PythonRecorder&, const PythonRecorder& );
  inline friend PythonRecorder operator + ( double lhs, const PythonRecorder& rhs) { return PythonRecorder(lhs)+rhs; }
  inline friend PythonRecorder operator + ( const PythonRecorder& lhs, double rhs)  { return lhs+PythonRecorder(rhs); }
  friend DLL_EXPORT PythonRecorder operator - ( const PythonRecorder&, const PythonRecorder& );
  inline friend PythonRecorder operator - ( const PythonRecorder& lhs, double rhs ) { return lhs-PythonRecorder(rhs); }
  inline friend PythonRecorder operator - ( double lhs, const PythonRecorder& rhs )  { return PythonRecorder(lhs)-rhs; }
  friend DLL_EXPORT PythonRecorder operator * ( const PythonRecorder&, const PythonRecorder& );
  inline friend PythonRecorder operator * ( double lhs, const PythonRecorder& rhs)  { return PythonRecorder(lhs)*rhs; }
  inline friend PythonRecorder operator * ( const PythonRecorder& lhs, double rhs) { return lhs*PythonRecorder(rhs); }
  inline friend PythonRecorder operator / ( const PythonRecorder& lhs, double rhs) { return lhs/PythonRecorder(rhs); }
  friend DLL_EXPORT PythonRecorder operator / ( const PythonRecorder&, const PythonRecorder& );
  friend PythonRecorder operator / ( double lhs, const PythonRecorder& rhs )  { return PythonRecorder(lhs)/rhs; }

  /* Unary operators */
  friend PythonRecorder DLL_EXPORT exp  ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT log  ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT sqrt ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT sin  ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT cos  ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT tan  ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT asin ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT acos ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT atan ( const PythonRecorder& );

  /* Additional functions */
  friend PythonRecorder DLL_EXPORT sinh  ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT cosh  ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT tanh  ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT asinh ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT acosh ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT atanh ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT erf   ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT fabs  ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT ceil  ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT floor ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT fmax ( const PythonRecorder&, const PythonRecorder& );
  inline friend PythonRecorder fmax ( double lhs, const PythonRecorder& rhs) { return fmax(PythonRecorder(lhs), rhs); }
  inline friend PythonRecorder fmax ( const PythonRecorder& lhs, double rhs) { return fmax(lhs, PythonRecorder(rhs)); }
  friend PythonRecorder DLL_EXPORT fmin ( const PythonRecorder&, const PythonRecorder& );
  inline friend PythonRecorder fmin ( double lhs, const PythonRecorder& rhs) { return fmin(PythonRecorder(lhs), rhs); }
  inline friend PythonRecorder fmin ( const PythonRecorder& lhs, double rhs) { return fmin(lhs, PythonRecorder(rhs)); }

  /* Special operators */
  friend PythonRecorder DLL_EXPORT atan2 ( const PythonRecorder&, const PythonRecorder& );
  friend DLL_EXPORT PythonRecorder log10 ( const PythonRecorder& );
  friend PythonRecorder DLL_EXPORT pow ( const PythonRecorder&, const PythonRecorder& );
  inline friend PythonRecorder pow ( double lhs, const PythonRecorder& rhs) { return pow(PythonRecorder(lhs), rhs); }
  inline friend PythonRecorder pow ( const PythonRecorder& lhs, double rhs) { return pow(lhs, PythonRecorder(rhs)); }

protected:
  void disp(std::ostream &stream) const;
  static int get_id();
  bool is_symbol() const;
  std::string repr() const;
  static PythonRecorder from_binary(const PythonRecorder& lhs, const PythonRecorder& rhs, double res, const std::string& op);
  static PythonRecorder from_unary(const PythonRecorder& arg, double res, const std::string& op);

  static std::ofstream& stream();
  explicit PythonRecorder(double value, int id);
  double value_;
  int id_;

  static int counter;
  static int counter_input;
  static int counter_output;
  static int counter_bool;

private:
  static std::unique_ptr<PythonStreamWrapper> python_stream_wrapper_;
};


class DLL_EXPORT MatlabRecorder{
public:
  ~MatlabRecorder();
  MatlabRecorder();
  MatlabRecorder(double value);
  void operator<<=(double value);
  void operator>>=(double& value);
  explicit operator bool() const;
  MatlabRecorder(const MatlabRecorder& r);
  friend DLL_EXPORT std::ostream& operator<<(std::ostream &stream, const MatlabRecorder& obj);
  static void stop_recording();
  static void start_recording();
  static void start_recording(const std::string& filename);

  /* Assignments */
  double getValue() const;
  inline double value() const {return getValue();}
  inline MatlabRecorder& operator = ( double arg) { return operator=(MatlabRecorder(arg)); }
  MatlabRecorder& operator = ( const MatlabRecorder& );

  /* IO friends */
  friend DLL_EXPORT std::istream& operator >> (std::istream& is, const MatlabRecorder& a);

  /* Operation and assignment */
  inline MatlabRecorder& operator += ( double value ) { return operator+=(MatlabRecorder(value)); }
  inline MatlabRecorder& operator += ( const MatlabRecorder& value) { return operator=(*this+value); }
  inline MatlabRecorder& operator -= ( double value ) { return operator-=(MatlabRecorder(value)); }
  inline MatlabRecorder& operator -= ( const MatlabRecorder& value) { return operator=(*this-value); }
  inline MatlabRecorder& operator *= ( double value)  { return operator*=(MatlabRecorder(value)); }
  inline MatlabRecorder& operator *= ( const MatlabRecorder& value) { return operator=(*this*value); }
  inline MatlabRecorder& operator /= ( double value)  { return operator/=(MatlabRecorder(value)); }
  inline MatlabRecorder& operator /= ( const MatlabRecorder& value) { return operator=(*this/value); }

  /* Comparison */
  friend bool DLL_EXPORT operator != ( const MatlabRecorder&, const MatlabRecorder& );
  friend bool DLL_EXPORT operator == ( const MatlabRecorder&, const MatlabRecorder& );
  friend bool DLL_EXPORT operator <= ( const MatlabRecorder&, const MatlabRecorder& );
  friend bool DLL_EXPORT operator >= ( const MatlabRecorder&, const MatlabRecorder& );
  friend bool DLL_EXPORT operator >  ( const MatlabRecorder&, const MatlabRecorder& );
  friend bool DLL_EXPORT operator <  ( const MatlabRecorder&, const MatlabRecorder& );
  inline friend bool operator != (double lhs, const MatlabRecorder& rhs) { return MatlabRecorder(lhs)!=rhs; }
  inline friend bool operator == ( double lhs, const MatlabRecorder& rhs) { return MatlabRecorder(lhs)==rhs; }
  inline friend bool operator <= ( double lhs, const MatlabRecorder& rhs) { return MatlabRecorder(lhs)<=rhs; }
  inline friend bool operator >= ( double lhs, const MatlabRecorder& rhs) { return MatlabRecorder(lhs)>=rhs; }
  inline friend bool operator >  ( double lhs, const MatlabRecorder& rhs) { return MatlabRecorder(lhs)>rhs; }
  inline friend bool operator <  ( double lhs, const MatlabRecorder& rhs) { return MatlabRecorder(lhs)<rhs; }

  /* Sign operators */
  inline friend MatlabRecorder operator + ( const MatlabRecorder& x ) { return x; }
  friend MatlabRecorder DLL_EXPORT  operator - ( const MatlabRecorder& x );

  /* Binary operators */
  friend MatlabRecorder DLL_EXPORT operator + ( const MatlabRecorder&, const MatlabRecorder& );
  inline friend MatlabRecorder operator + ( double lhs, const MatlabRecorder& rhs) { return MatlabRecorder(lhs)+rhs; }
  inline friend MatlabRecorder operator + ( const MatlabRecorder& lhs, double rhs)  { return lhs+MatlabRecorder(rhs); }
  friend DLL_EXPORT MatlabRecorder operator - ( const MatlabRecorder&, const MatlabRecorder& );
  inline friend MatlabRecorder operator - ( const MatlabRecorder& lhs, double rhs ) { return lhs-MatlabRecorder(rhs); }
  inline friend MatlabRecorder operator - ( double lhs, const MatlabRecorder& rhs )  { return MatlabRecorder(lhs)-rhs; }
  friend DLL_EXPORT MatlabRecorder operator * ( const MatlabRecorder&, const MatlabRecorder& );
  inline friend MatlabRecorder operator * ( double lhs, const MatlabRecorder& rhs)  { return MatlabRecorder(lhs)*rhs; }
  inline friend MatlabRecorder operator * ( const MatlabRecorder& lhs, double rhs) { return lhs*MatlabRecorder(rhs); }
  inline friend MatlabRecorder operator / ( const MatlabRecorder& lhs, double rhs) { return lhs/MatlabRecorder(rhs); }
  friend DLL_EXPORT MatlabRecorder operator / ( const MatlabRecorder&, const MatlabRecorder& );
  friend MatlabRecorder operator / ( double lhs, const MatlabRecorder& rhs )  { return MatlabRecorder(lhs)/rhs; }

  /* Unary operators */
  friend MatlabRecorder DLL_EXPORT exp  ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT log  ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT sqrt ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT sin  ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT cos  ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT tan  ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT asin ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT acos ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT atan ( const MatlabRecorder& );

  /* Additional functions */
  friend MatlabRecorder DLL_EXPORT sinh  ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT cosh  ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT tanh  ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT asinh ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT acosh ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT atanh ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT erf   ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT fabs  ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT ceil  ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT floor ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT fmax ( const MatlabRecorder&, const MatlabRecorder& );
  inline friend MatlabRecorder fmax ( double lhs, const MatlabRecorder& rhs) { return fmax(MatlabRecorder(lhs), rhs); }
  inline friend MatlabRecorder fmax ( const MatlabRecorder& lhs, double rhs) { return fmax(lhs, MatlabRecorder(rhs)); }
  friend MatlabRecorder DLL_EXPORT fmin ( const MatlabRecorder&, const MatlabRecorder& );
  inline friend MatlabRecorder fmin ( double lhs, const MatlabRecorder& rhs) { return fmin(MatlabRecorder(lhs), rhs); }
  inline friend MatlabRecorder fmin ( const MatlabRecorder& lhs, double rhs) { return fmin(lhs, MatlabRecorder(rhs)); }

  /* Special operators */
  friend MatlabRecorder DLL_EXPORT atan2 ( const MatlabRecorder&, const MatlabRecorder& );
  friend DLL_EXPORT MatlabRecorder log10 ( const MatlabRecorder& );
  friend MatlabRecorder DLL_EXPORT pow ( const MatlabRecorder&, const MatlabRecorder& );
  inline friend MatlabRecorder pow ( double lhs, const MatlabRecorder& rhs) { return pow(MatlabRecorder(lhs), rhs); }
  inline friend MatlabRecorder pow ( const MatlabRecorder& lhs, double rhs) { return pow(lhs, MatlabRecorder(rhs)); }

protected:
  void disp(std::ostream &stream) const;
  static int get_id();
  bool is_symbol() const;
  std::string repr() const;
  static MatlabRecorder from_binary(const MatlabRecorder& lhs, const MatlabRecorder& rhs, double res, const std::string& op);
  static MatlabRecorder from_unary(const MatlabRecorder& arg, double res, const std::string& op);

  static std::ofstream& stream();
  explicit MatlabRecorder(double value, int id);
  double value_;
  int id_;

  static int counter;
  static int counter_input;
  static int counter_output;
  static int counter_bool;

private:
  static std::unique_ptr<MatlabStreamWrapper> matlab_stream_wrapper_;
};

#endif // Recorder_H_
