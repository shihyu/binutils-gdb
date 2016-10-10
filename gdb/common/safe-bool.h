/* Safe bool idiom implementation.

   Copyright (C) 2016 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef SAFE_BOOL_H
#define SAFE_BOOL_H 1

/* Helper classes used to implement the safe bool idiom, for compilers
   that don't support "explicit operator bool()" (C++11).  Classes
   that want to support explicit boolean conversion inherit from
   safe_bool (using CRTP) and implement the explicit_operator_bool
   method.  */

class safe_bool_base
{
protected:
  typedef void (safe_bool_base::*bool_type) () const;
  void this_type_does_not_support_comparisons () const {}
};

/* Baseclass, using CRTP.  */

template <typename T = void>
class safe_bool : public safe_bool_base {
public:
  operator bool_type () const
  {
    return (static_cast<const T *>(this))->explicit_operator_bool ()
      ? &safe_bool_base::this_type_does_not_support_comparisons : 0;
  }

protected:
  ~safe_bool () {}
};

/* Comparison operators.  */

template <typename T, typename U>
bool operator== (const safe_bool<T> &lhs, const safe_bool<U> &rhs)
{
  lhs.this_type_does_not_support_comparisons ();
  return false;
}

template <typename T,typename U>
bool operator!= (const safe_bool<T> &lhs, const safe_bool<U> &rhs)
{
  lhs.this_type_does_not_support_comparisons ();
  return false;
}

#endif /* SAFE_BOOL_H */
