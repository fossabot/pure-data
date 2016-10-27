#include <m_pd.h>
#include "ceammc.h"

t_class* is_list_class;
struct t_is_list {
    t_object x_obj;
};

static void is_list_anything(t_is_list *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_float(x->x_obj.te_outlet, 0);
}

static void is_list_bang(t_is_list* x)
{
    outlet_float(x->x_obj.te_outlet, 0);
}

static void is_list_float(t_is_list* x, t_floatarg f)
{
    outlet_float(x->x_obj.te_outlet, 0);
}

static void is_list_list(t_is_list* x, t_symbol* s, int argc, t_atom* argv)
{
    outlet_float(x->x_obj.te_outlet, 1);
}

static void is_list_pointer(t_is_list* x, t_gpointer *pt)
{
     outlet_float(x->x_obj.te_outlet, 0);
}

static void is_list_symbol(t_is_list* x, t_symbol *s)
{
    outlet_float(x->x_obj.te_outlet, 0);
}

static void* is_list_new()
{
    t_is_list* x = reinterpret_cast<t_is_list*>(pd_new(is_list_class));
    outlet_new(&x->x_obj, &s_float);
    
    return static_cast<void*>(x);
}

extern "C" void setup_is0x2elist()
{
    is_list_class = class_new(gensym("is.list"),
        reinterpret_cast<t_newmethod>(is_list_new),
        reinterpret_cast<t_method>(0),
        sizeof(t_is_list), 0, A_NULL);
    class_addanything(is_list_class, is_list_anything);
    class_addbang(is_list_class, is_list_bang);
    class_addfloat(is_list_class, is_list_float);
    class_addlist(is_list_class, is_list_list);
    class_addpointer(is_list_class, is_list_pointer);
    class_addsymbol(is_list_class, is_list_symbol);
}

