// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

#define LUABIND_BUILDING

#include <luabind/lua_include.hpp>

#include <luabind/luabind.hpp>
#include <luabind/class_info.hpp>
#include <luabind/detail/class_registry.hpp>
#include <luabind/detail/class_rep.hpp>

namespace luabind
{
	LUABIND_API class_info get_class_info(argument const& o)
	{
		lua_State* L = o.interpreter();
	
		o.push(L);
        detail::object_rep* obj = detail::get_instance(L, -1);

        if (!obj)
        {
            class_info result;
            result.name = lua_typename(L, lua_type(L, -1));
            lua_pop(L, 1);
            result.methods = newtable(L);
            result.attributes = newtable(L);
            result.static_methods = newtable(L);
            result.constants = newtable(L);
            result.bases = newtable(L);
            if(detail::is_class_rep(L,-1)){
                detail::class_rep* crep = static_cast<detail::class_rep*>(lua_touserdata(L, -1));
                crep->get_table(L);
                object table(from_stack(L, -1));
                lua_pop(L, 1);

                result.name = crep->name();

                std::size_t index = 1;

                for (iterator i(table), e; i != e; ++i)
                {
                    if (type(*i) != LUA_TFUNCTION)
                        continue;

                    // We have to create a temporary `object` here, otherwise the proxy
                    // returned by operator->() will mess up the stack. This is a known
                    // problem that probably doesn't show up in real code very often.
                    object member(*i);
                    member.push(L);
                    detail::stack_pop pop(L, 1);

                    if (lua_tocfunction(L, -1) == &detail::property_tag)
                    {
                        result.attributes[index++] = i.key();
                    }
                    else
                    {
                        result.methods[i.key()] = *i;
                    }
                }

                const std::map<const char*, int, detail::ltstr>& constants = crep->static_constants();
                std::map<const char*, int, detail::ltstr>::const_iterator it = constants.begin();
                index = 1;
                for(; it != constants.end(); it++){
                    result.constants[it->first] = it->second;
                }
                crep->get_default_table(L);
                object def_table(from_stack(L, -1));
                lua_pop(L, 1);
                for (iterator i(def_table), e; i != e; ++i){

                    object member(*i);
                    member.push(L);
                    detail::stack_pop pop(L, 1);

                    result.static_methods[i.key()] = *i;
                }
                index = 1;
                const std::vector<detail::class_rep::base_info>& bases = crep->bases();
                std::vector<detail::class_rep::base_info>::const_iterator i2 = bases.begin();
                std::string names("base classes: ");
                for(; i2 != bases.end(); i2++){
                    result.bases[index++] = i2->base->name();
                }
            }
            return result;
        }

        lua_pop(L, 1);

        obj->crep()->get_table(L);
        object table(from_stack(L, -1));
        lua_pop(L, 1);

        class_info result;
        result.name = obj->crep()->name();
        result.methods = newtable(L);
        result.attributes = newtable(L);
        result.static_methods = newtable(L);
        result.constants = newtable(L);
        result.bases = newtable(L);

        std::size_t index = 1;

        for (iterator i(table), e; i != e; ++i)
        {
            if (type(*i) != LUA_TFUNCTION)
                continue;

            // We have to create a temporary `object` here, otherwise the proxy
            // returned by operator->() will mess up the stack. This is a known
            // problem that probably doesn't show up in real code very often.
            object member(*i);
            member.push(L);
            detail::stack_pop pop(L, 1);

            if (lua_tocfunction(L, -1) == &detail::property_tag)
            {
                result.attributes[index++] = i.key();
            }
            else
            {
                result.methods[i.key()] = *i;
            }
        }

        obj->crep()->get_default_table(L);
        object def_table(from_stack(L, -1));
        lua_pop(L, 1);
        for (iterator i(def_table), e; i != e; ++i){
            object member(*i);
            member.push(L);
            detail::stack_pop pop(L, 1);
            result.static_methods[i.key()] = *i;
        }

        const std::map<const char*, int, detail::ltstr>& constants = obj->crep()->static_constants();
        std::map<const char*, int, detail::ltstr>::const_iterator it = constants.begin();
        index = 1;
        for(; it != constants.end(); it++){
            result.constants[it->first] = it->second;
        }

        index = 1;
        const std::vector<detail::class_rep::base_info>& bases = obj->crep()->bases();
        std::vector<detail::class_rep::base_info>::const_iterator i2 = bases.begin();
        for(; i2 != bases.end(); i2++){
            result.bases[index++] = i2->base->name();
        }

        return result;
	}

    LUABIND_API object get_class_names(lua_State* L)
    {
        detail::class_registry* reg = detail::class_registry::get_registry(L);

        std::map<type_id, detail::class_rep*> const& classes = reg->get_classes();

        object result = newtable(L);
        std::size_t index = 1;

        for (std::map<type_id, detail::class_rep*>::const_iterator iter = classes.begin();
            iter != classes.end(); ++iter)
        {
            result[index++] = iter->second->name();
        }

        return result;
    }

	LUABIND_API void bind_class_info(lua_State* L)
	{
		module(L)
		[
			class_<class_info>("class_info_data")
				.def_readonly("name", &class_info::name)
				.def_readonly("methods", &class_info::methods)
                                .def_readonly("attributes", &class_info::attributes)
                                .def_readonly("constants", &class_info::constants)
                                .def_readonly("static_methods", &class_info::static_methods)
                                .def_readonly("bases", &class_info::bases),
		
            def("class_info", &get_class_info),
            def("class_names", &get_class_names)
		];
	}
}

