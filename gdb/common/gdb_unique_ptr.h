/* gdb::unique_ptr, a simple std::unique_ptr replacement for C++03.

   Copyright (C) 2007-2016 Free Software Foundation, Inc.

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

/* gdb::unique_ptr maps to std::unique_ptr in C++11 mode, and to a
   simplified emulation in C++03 mode.

   The emulation code was originally based on GCC 7.0's std::auto_ptr
   and then heavily customized to behave more like std::unique_ptr
   (T[] specialization, assignment from NULL, explicit bool
   conversion, etc.).

   Our emulation actually lets you shoot yourself in the foot, just
   like std::auto_ptr, since the copy ctor actually moves, but we know
   that if gdb builds in C++11 mode, then we're not doing anything
   unsafe.

   Note, our emulation does NOT support "stateful" custom deleters.
   You can't pass a deleter argument to the constructor.  Only the
   managed pointer is stored.  Turns out that we don't really need
   stateful custom deleters in practice.

   At the end of the file you'll find a gdb::unique_ptr specialization
   that uses a custom (stateless) deleter -- gdb::unique_malloc_ptr.
*/

#ifndef GDB_UNIQUE_PTR_H
#define GDB_UNIQUE_PTR_H 1

#include <memory>

#include "safe-bool.h"

namespace gdb
{

#if __cplusplus >= 201103

/* In C++ mode, all we need is import the standard
   std::unique_ptr.  */
template<typename T> using unique_ptr
  = std::unique_ptr<T>;

/* Pull in move as well.  */
using std::move;

#else /* C++11 */

/* Default destruction policy used by gdb::unique_ptr when no
   deleter is specified.  Uses delete.  */

template<typename T>
struct default_delete
{
  void operator () (T *ptr) const { delete ptr; }
};

/* Specialization for arrays.  Uses delete[].  */

template<typename T>
struct default_delete<T[]>
{
  void operator () (T *ptr) const { delete [] ptr; }
};

/* Type used to support assignment from NULL:

     gdb::unique_ptr<foo> ptr (....);
     ...
     ptr = NULL;
*/
struct unique_ptr_nullptr_t
{
private:
  struct private_type;
public:
  /* Since null_type is private, the only way to construct this class
     is by passing a NULL pointer.  See unique_ptr_base::operator=
     further below.  */
  unique_ptr_nullptr_t (private_type *) {}
};

/* Base class of our unique_ptr emulation.  Contains code common to
   both the unique_ptr<T, D> and unique_ptr<T[], D>.  */

template<typename T, typename D>
class unique_ptr_base : public safe_bool <unique_ptr_base<T, D> >
{
public:
  typedef T *pointer;
  typedef T element_type;
  typedef D deleter_type;

  template <typename T1>
  struct unique_ptr_base_ref
  {
    T1 *m_ptr;

    explicit unique_ptr_base_ref (T1 *p): m_ptr (p) {}
  };

  typedef unique_ptr_base_ref<T> ref_type;

  /* An unique_ptr is usually constructed from a raw pointer.  P - a
     pointer (defaults to NULL).  This object now owns the object
     pointed to by P.  */
  explicit unique_ptr_base (element_type *p = NULL) throw() : m_ptr (p) {}

  /* Even though std::unique_ptr is not copyable, our little simpler
     emulation allows it, because RVO/NRVO requires an accessible copy
     constructor, and also because our move emulation relies on this.

     An unique_ptr_base can be constructed from another
     unique_ptr_base.  A is another unique_ptr_base of the same type.

     This object now owns the object previously owned by A, which has
     given up ownership.  */
  unique_ptr_base (unique_ptr_base& a) throw() : m_ptr (a.release ()) {}

  /* Similarly, we implement this simply to allow std::swap work
     without having to provide our own implementation.  We know that
     if GDB compiles with real std::unique_ptr, this won't be called
     "incorrectly".

     Assignment operator.  A is another unique_ptr_base of the same
     type.  This object now owns the object previously owned by A,
     which has given up ownership.  The object that this one used to
     own and track has been deleted.  */
  unique_ptr_base&
  operator= (unique_ptr_base &a) throw()
  {
    reset (a.release ());
    return *this;
  }

  /* std::unique_ptr does not allow assignment, except from nullptr.
     nullptr doesn't exist before C++11, so we allowing assignment
     from NULL instead:
       ptr = NULL;
  */
  unique_ptr_base &
  operator= (const unique_ptr_nullptr_t &) throw()
  {
    reset ();
    return *this;
  }

  /* When the unique_ptr_base goes out of scope, the object it owns is
     deleted.  If it no longer owns anything (i.e., get() is NULL,
     then this has no effect.  */
  ~unique_ptr_base () { call_deleter (); }

  /* "explicit operator bool" emulation using the safe bool idiom.  */
  bool explicit_operator_bool () const
  {
    return m_ptr != NULL;
  }

  /* Bypassing the smart pointer.
     Returns the raw pointer being managed.

     You can get a copy of the pointer that this object owns, for
     situations such as passing to a function which only accepts a raw
     pointer.  Note, this smarts pointer still owns the memory.  */
  element_type *get () const throw() { return m_ptr; }

  /* Bypassing the smart pointer.  Returns the raw pointer being
     managed.

     You can get a copy of the pointer that this object owns, for
     situations such as passing to a function which only accepts a raw
     pointer.

     Note, this smart pointer no longer owns the memory.  When this
     object goes out of scope, nothing will happen.  */
  element_type *release () throw()
  {
    pointer tmp = m_ptr;
    m_ptr = NULL;
    return tmp;
  }

  /* Forcibly delete the managed object.  P is a pointer (defaults to
     NULL).  This object now owns the object pointed to by P.  The
     previous object has been deleted.  */
  void reset (element_type *p = NULL) throw()
  {
    if (p != m_ptr)
      {
	call_deleter ();
	m_ptr = p;
      }
  }

  /* Automatic conversions.

     These operations convert an unique_ptr_base into and from an
     unique_ptr_base_ref automatically as needed.  This allows
     constructs such as:

      unique_ptr<Derived>  func_returning_unique_ptr (.....);
      ...
      unique_ptr<Base> ptr = func_returning_unique_ptr (.....);
   */
  unique_ptr_base (unique_ptr_base_ref<element_type> ref) throw()
    : m_ptr (ref.m_ptr) {}

  unique_ptr_base &
  operator= (unique_ptr_base_ref<element_type> ref) throw()
  {
    if (ref.m_ptr != this->get())
      {
	call_deleter ();
	m_ptr = ref.m_ptr;
      }
    return *this;
  }

  template<typename T1>
  operator unique_ptr_base_ref<T1> () throw()
  { return unique_ptr_base_ref<T1> (this->release ()); }

  template<typename T1, typename D1>
  operator unique_ptr_base<T1, D1> () throw()
  { return unique_ptr_base<T1, D1> (this->release ()); }

private:

  /* Call the deleter.  Note we assume the deleter is "stateless".  */
  void call_deleter ()
  {
    D d;

    d (m_ptr);
  }

  element_type *m_ptr;
};

/* Macro used to create a unique_ptr_base "specialization" -- a
   subclass that uses a specific deleter.  Basically this re-defines
   the necessary constructors.  This is necessary because we can't
   inherit constructors with "using" without C++11.  While at it, we
   inherit the assignment operator.  TYPE is the name of the type
   being defined.  Assumes that 'base_type' is a typedef of the
   baseclass UNIQUE_PTR is inheriting from.  */
#define DEFINE_UNIQUE_PTR(TYPE)					\
  public:								\
  explicit TYPE (T *p = NULL) throw()					\
    : base_type (p) {}							\
  TYPE (typename base_type::ref_type ref) throw()			\
    : base_type (ref.m_ptr) {}						\
									\
  using base_type::operator=;

/* Finally, define gdb::unique_ptr.  */

template <typename T, typename D = default_delete<T> >
class unique_ptr : public unique_ptr_base<T, D>
{
  typedef unique_ptr_base<T, D> base_type;

  DEFINE_UNIQUE_PTR (unique_ptr)

  /* Dereferencing.  */
  T &operator* () const throw() { return *this->get (); }
  T *operator-> () const throw() { return this->get (); }
};

/* gdb::unique_ptr specialization for T[].  */

template <typename T, typename D>
class unique_ptr<T[], D> : public unique_ptr_base<T, D>
{
  typedef unique_ptr_base<T, D> base_type;

  DEFINE_UNIQUE_PTR (unique_ptr)

  /* Indexing operator.  */
  T& operator[] (size_t i) const
  { return this->get ()[i]; }
};

/* Comparison operators.  */

template <typename T, typename D,
	  typename U, typename E>
inline bool
operator== (const unique_ptr_base<T, D>& x,
	    const unique_ptr_base<U, E>& y)
{ return x.get() == y.get(); }

template <typename T, typename D,
	  typename U, typename E>
inline bool
operator!= (const unique_ptr_base<T, D>& x,
	    const unique_ptr_base<U, E>& y)
{ return x.get() != y.get(); }

/* std::move "emulation".  This is as simple as it can be -- relies on
   the fact that our std::unique_ptr emulation actually behaves like
   std::auto_ptr -- copy/assignment actually moves.  */

template<typename T, typename D>
unique_ptr_base<T, D>
move (unique_ptr_base<T, D> v)
{
  return v;
}

#endif /* C++11 */

/* Define gdb::unique_malloc_ptr, a gdb::unique_ptr that manages
   malloc'ed memory.  */

/* The deleter for gdb::unique_malloc_ptr.  Uses xfree.  */
template <typename T>
struct xfree_deleter
{
  void operator() (T *ptr) const { xfree (ptr); }
};

#if __cplusplus >= 201103

/* In C++11, we just import the standard unique_ptr to our
   namespace with a custom deleter.  */

template<typename T> using unique_malloc_ptr
  = std::unique_ptr<T, xfree_deleter<T>>;

#else /* C++11 */

/* In C++03 mode, we need to define a subclass instead (and re-define
   the constructors).  */

template <typename T>
class unique_malloc_ptr : public unique_ptr<T, xfree_deleter<T> >
{
  typedef unique_ptr<T, xfree_deleter<T> > base_type;

  DEFINE_UNIQUE_PTR (unique_malloc_ptr)
};

#endif /* C++11 */

} /* namespace gdb */

#endif /* GDB_UNIQUE_PTR_H */
