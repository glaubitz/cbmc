/*******************************************************************\

Module: Expressions in JSON

Author: Peter Schrammel

\*******************************************************************/

#include "namespace.h"
#include "expr.h"
#include "json.h"
#include "arith_tools.h"
#include "ieee_float.h"
#include "fixedbv.h"
#include "std_expr.h"
#include "config.h"
#include "i2string.h"

#include "json_expr.h"

/*******************************************************************\

Function: json

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

json_objectt json(const source_locationt &location)
{
  json_objectt result;
  
  if(!location.get_file().empty())
    result["file"]=json_stringt(id2string(location.get_file()));

  if(!location.get_line().empty())
    result["line"]=json_stringt(id2string(location.get_line()));

  if(!location.get_column().empty())
    result["column"]=json_stringt(id2string(location.get_column()));

  if(!location.get_function().empty())
    result["function"]=json_stringt(id2string(location.get_function()));
  
  return result;
}

/*******************************************************************\

Function: json

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

json_objectt json(
  const typet &type,
  const namespacet &ns)
{
  if(type.id()==ID_symbol)
    return json(ns.follow(type), ns);

  json_objectt result;

  if(type.id()==ID_unsignedbv)
  {
    result["name"]=json_stringt("integer");
    result["width"]=
      json_numbert(i2string(to_unsignedbv_type(type).get_width()));
  }
  else if(type.id()==ID_signedbv)
  {
    result["name"]=json_stringt("integer");
    result["width"]=json_numbert(i2string(to_signedbv_type(type).get_width()));
  }
  else if(type.id()==ID_floatbv)
  {
    result["name"]=json_stringt("float");
    result["width"]=json_numbert(i2string(to_floatbv_type(type).get_width()));
  }
  else if(type.id()==ID_bv)
  {
    result["name"]=json_stringt("integer");
    result["width"]=json_numbert(i2string(to_bv_type(type).get_width()));
  }
  else if(type.id()==ID_c_bit_field)
  {
    result["name"]=json_stringt("integer");
    result["width"]=
      json_numbert(i2string(to_c_bit_field_type(type).get_width()));
  }
  else if(type.id()==ID_c_enum_tag)
  {
    // we return the base type
    return json(ns.follow_tag(to_c_enum_tag_type(type)).subtype(), ns);
  }
  else if(type.id()==ID_fixedbv)
  {
    result["name"]=json_stringt("fixed");
    result["width"]=json_numbert(i2string(to_fixedbv_type(type).get_width()));
  }
  else if(type.id()==ID_pointer)
  {
    result["name"]=json_stringt("pointer");
    result["subtype"]=json(type.subtype(), ns);
  }
  else if(type.id()==ID_bool)
  {
    result["name"]=json_stringt("boolean");
  }
  else if(type.id()==ID_array)
  {
    result["name"]=json_stringt("array");
    result["subtype"]=json(type.subtype(), ns);
  }
  else if(type.id()==ID_vector)
  {
    result["name"]=json_stringt("vector");
    result["subtype"]=json(type.subtype(), ns);
    result["size"]=json(to_vector_type(type).size(), ns);
  }
  else if(type.id()==ID_struct)
  {
    result["name"]=json_stringt("struct");
    json_arrayt &members=result["members"].make_array();
    const struct_typet::componentst &components=
      to_struct_type(type).components();
    for(const auto & it : components)
    {
      json_objectt &e=members.push_back().make_object();
      e["name"]=json_stringt(id2string(it.get_name()));
      e["type"]=json(it.type(), ns);
    }
  }
  else if(type.id()==ID_union)
  {
    result["name"]=json_stringt("union");
    json_arrayt &members=result["members"].make_array();
    const union_typet::componentst &components=
      to_union_type(type).components();
    for(const auto & it : components)
    {
      json_objectt &e=members.push_back().make_object();
      e["name"]=json_stringt(id2string(it.get_name()));
      e["type"]=json(it.type(), ns);
    }
  }
  else
    result["name"]=json_stringt("unknown");

  return result;
}

/*******************************************************************\

Function: json

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

json_objectt json(
  const exprt &expr,
  const namespacet &ns)
{
  json_objectt result;
  
  const typet &type=ns.follow(expr.type());

  if(expr.id()==ID_constant)
  {
    if(type.id()==ID_unsignedbv ||
       type.id()==ID_signedbv ||
       type.id()==ID_c_bit_field)
    {
      std::size_t width=to_bitvector_type(type).get_width();
    
      result["name"]=json_stringt("integer");
      result["binary"]=json_stringt(expr.get_string(ID_value));
      result["width"]=json_numbert(i2string(width));
      
      const typet &underlying_type=
        type.id()==ID_c_bit_field?type.subtype():
        type;

      bool is_signed=underlying_type.id()==ID_signedbv;
        
      std::string sig=is_signed?"":"unsigned ";

      if(width==config.ansi_c.char_width)
        result["c_type"]=json_stringt(sig+"char");
      else if(width==config.ansi_c.int_width)
        result["c_type"]=json_stringt(sig+"int");
      else if(width==config.ansi_c.short_int_width)
        result["c_type"]=json_stringt(sig+"short int");
      else if(width==config.ansi_c.long_int_width)
        result["c_type"]=json_stringt(sig+"long int");
      else if(width==config.ansi_c.long_long_int_width)
        result["c_type"]=json_stringt(sig+"long long int");

      mp_integer i;
      if(!to_integer(expr, i))
        result["data"]=json_stringt(integer2string(i));
    }
    else if(type.id()==ID_c_enum)
    {
      result["name"]=json_stringt("integer");
      result["binary"]=json_stringt(expr.get_string(ID_value));
      result["width"]=json_numbert(type.subtype().get_string(ID_width));
      result["c_type"]=json_stringt("enum");

      mp_integer i;
      if(!to_integer(expr, i))
        result["data"]=json_stringt(integer2string(i));
    }
    else if(type.id()==ID_c_enum_tag)
    {
      constant_exprt tmp;
      tmp.type()=ns.follow_tag(to_c_enum_tag_type(type));
      tmp.set_value(to_constant_expr(expr).get_value());
      return json(tmp, ns);
    }
    else if(type.id()==ID_bv)
    {
      result["name"]=json_stringt("bitvector");
      result["binary"]=json_stringt(expr.get_string(ID_value));
    }
    else if(type.id()==ID_fixedbv)
    {
      result["name"]=json_stringt("fixed");
      result["width"]=json_numbert(type.get_string(ID_width));
      result["binary"]=json_stringt(expr.get_string(ID_value));
      result["data"]=
        json_stringt(fixedbvt(to_constant_expr(expr)).to_ansi_c_string());
    }
    else if(type.id()==ID_floatbv)
    {
      result["name"]=json_stringt("float");
      result["width"]=json_numbert(type.get_string(ID_width));
      result["binary"]=json_stringt(expr.get_string(ID_value));
      result["data"]=
        json_stringt(ieee_floatt(to_constant_expr(expr)).to_ansi_c_string());
    }
    else if(type.id()==ID_pointer)
    {
      result["name"]=json_stringt("pointer");
      result["binary"]=json_stringt(expr.get_string(ID_value));
      if(expr.get(ID_value)==ID_NULL)
        result["data"]=json_stringt("NULL");
    }
    else if(type.id()==ID_bool)
    {
      result["name"]=json_stringt("boolean");
      result["binary"]=json_stringt(expr.is_true()?"1":"0");
      result["data"]=jsont::json_boolean(expr.is_true());
    }
    else
    {
      result["name"]=json_stringt("unknown");
    }
  }
  else if(expr.id()==ID_array)
  {
    result["name"]=json_stringt("array");
    json_arrayt &elements=result["elements"].make_array();
    
    unsigned index=0;
    
    forall_operands(it, expr)
    {
      json_objectt &e=elements.push_back().make_object();
      e["index"]=json_numbert(i2string(index));
      e["value"]=json(*it, ns);
      index++;
    }
  }
  else if(expr.id()==ID_struct)
  {
    result["name"]=json_stringt("struct");
    
    // these are expected to have a struct type
    if(type.id()==ID_struct)
    {
      const struct_typet &struct_type=to_struct_type(type);
      const struct_typet::componentst &components=struct_type.components();
      assert(components.size()==expr.operands().size());

      json_arrayt &members=result["members"].make_array();
      for(unsigned m=0; m<expr.operands().size(); m++)
      {
        json_objectt &e=members.push_back().make_object();
        e["value"]=json(expr.operands()[m], ns);
        e["name"]=json_stringt(id2string(components[m].get_name()));
      }
    }
  }
  else if(expr.id()==ID_union)
  { 
    result["name"]=json_stringt("union");
    
    assert(expr.operands().size()==1);
    
    json_objectt &e=result["member"].make_object();
    e["value"]=json(expr.op0(), ns);
    e["name"]=json_stringt(id2string(to_union_expr(expr).get_component_name()));
  }
  else
    result["name"]=json_stringt("unknown");

  return result;
}

