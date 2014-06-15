/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_ANSI_C_C_STORAGE_SPEC_H
#define CPROVER_ANSI_C_C_STORAGE_SPEC_H

#include <util/expr.h>

class c_storage_spect
{
public:
  c_storage_spect()
  {
    clear();
  }
  
  explicit c_storage_spect(const typet &type)
  {
    clear();
    read(type);
  }
  
  void clear()
  {
    is_typedef=false;
    is_extern=false;
    is_thread_local=false;
    is_static=false;
    is_register=false;
    is_inline=false;
  }
  
  bool is_typedef, is_extern, is_static, is_register,
       is_inline, is_thread_local;
  
  std::string as_string() const;

  friend bool operator == (
    const c_storage_spect &a,
    const c_storage_spect &b)
  {
    return a.is_typedef==b.is_typedef &&
           a.is_extern==b.is_extern &&
           a.is_static==b.is_static &&
           a.is_register==b.is_register &&
           a.is_thread_local==b.is_thread_local &&
           a.is_inline==b.is_inline;
  }

  friend bool operator != (
    const c_storage_spect &a,
    const c_storage_spect &b)
  {
    return !(a==b);
  }

  friend c_storage_spect &operator |= (
    c_storage_spect &a,
    const c_storage_spect &b)
  {
    a.is_typedef      |=b.is_typedef;
    a.is_extern       |=b.is_extern;
    a.is_static       |=b.is_static;
    a.is_register     |=b.is_register;
    a.is_inline       |=b.is_inline;
    a.is_thread_local |=b.is_thread_local;
    
    return a;
  }
  
  void read(const typet &type)
  {
    if(type.id()==ID_merged_type)
    {
      forall_subtypes(it, type)
        read(*it);
    }
    else if(type.id()==ID_static)
      is_static=true;
    else if(type.id()==ID_thread_local)
      is_thread_local=true;
    else if(type.id()==ID_inline)
      is_inline=true;
    else if(type.id()==ID_extern)
      is_extern=true;
    else if(type.id()==ID_typedef)
      is_typedef=true;
    else if(type.id()==ID_register)
      is_register=true;
    else if(type.id()==ID_auto)
    {
      // ignore
    }
  }
};

#endif