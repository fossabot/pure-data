/*
 * CicmWrapper
 * Copyright (C) 2013 Pierre Guillot, CICM - Université Paris 8
 * All rights reserved.
 * Website  : https://github.com/CICM/CicmWrapper
 * Contacts : cicm.mshparisnord@gmail.com
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "eclass.h"
#include "ebox.h"
#include "ecommon.h"
#include "egraphics.h"
#include "eobj.h"
#include "epopup.h"

#define _(msg) msg

static t_symbol* SYM_DUMP = gensym("dump");
static t_symbol* SYM_GET_ALL_PROPS = gensym("@*?");
static t_symbol* SYM_DIALOG = gensym("dialog");

static t_symbol* SYM_MOUSE_ENTER = gensym("mouseenter");
static t_symbol* SYM_MOUSE_LEAVE = gensym("mouseleave");
static t_symbol* SYM_MOUSE_MOVE = gensym("mousemove");
static t_symbol* SYM_MOUSE_DOWN = gensym("mousedown");
static t_symbol* SYM_MOUSE_DRAG = gensym("mousedrag");
static t_symbol* SYM_MOUSE_UP = gensym("mouseup");
static t_symbol* SYM_MOUSE_WHEEL = gensym("mousewheel");
static t_symbol* SYM_MOUSE_DBL_CLICK = gensym("dblclick");
static t_symbol* SYM_KEY = gensym("key");
static t_symbol* SYM_KEY_FILTER = gensym("keyfilter");

static t_symbol* SYM_PAINT = gensym("paint");
static t_symbol* SYM_NOTIFY = gensym("notify");
static t_symbol* SYM_GET_DRAW_PARAMS = gensym("getdrawparams");
static t_symbol* SYM_OK_SIZE = gensym("oksize");
static t_symbol* SYM_SAVE = gensym("save");
static t_symbol* SYM_POPUP = gensym("popup");
static t_symbol* SYM_PRESET = gensym("preset");

static t_symbol* SYM_POS = gensym("pos");
static t_symbol* SYM_VIS = gensym("vis");
static t_symbol* SYM_ZOOM = gensym("zoom");

static t_symbol* SYM_ENTRY = gensym("entry");
static t_symbol* SYM_CHECKBUTTON = gensym("checkbutton");
static t_symbol* SYM_ON_OFF = gensym("onoff");
static t_symbol* SYM_MENU = gensym("menu");
static t_symbol* SYM_COLOR = gensym("color");
static t_symbol* SYM_RGB = gensym("rgb");
static t_symbol* SYM_RGBA = gensym("rgba");
static t_symbol* SYM_PATH = gensym("path");
static t_symbol* SYM_NUMBER = gensym("number");
static t_symbol* SYM_SPINBOX = gensym("spinbox");

static t_symbol* SYM_ANY = gensym("anything");
static t_symbol* SYM_DSP = gensym("dsp");
static t_symbol* SYM_DSP_ADD = gensym("dsp_add");
static t_symbol* SYM_DSP_ADD64 = gensym("dsp_add");

static const int CAT_BASE = 0;
static const int CAT_COLOR = 100;
static const int CAT_BOUNDS = 200;
static const int CAT_MIDI = 300;
static const int CAT_MISC = 400;

static void eclass_properties_dialog(t_eclass* c);
static void ewidget_init(t_eclass* c);
static void eclass_attr_ceammc_setter(t_object* x, t_symbol* s, int argc, t_atom* argv);

static t_class* tcl_version_class = 0;
typedef struct tcl_version {
    t_object x_obj;
    char minor;
    char major;
} t_tcl_version;

t_tcl_version* tcl_version_instance;
t_tcl_version* tcl_version_new()
{
    t_tcl_version* x = (t_tcl_version*)pd_new(tcl_version_class);
    if (x) {
        x->minor = 0;
        x->major = 0;
    }

    return x;
}

int egraphics_smooth()
{
    if (!tcl_version_class || !tcl_version_instance)
        return 0;

    if (tcl_version_instance->major == 8 && tcl_version_instance->minor == 6)
        return 1;
    else
        return 0;
}

void tcl_version_set(t_tcl_version* x, t_float v)
{
    float int_part;
    float frac_part = modff(v, &int_part);

    x->major = (char)int_part;
    x->minor = (char)roundf(frac_part * 10);

    if (x->minor != 6)
        post("[ceammc] TCL/Tk version is: %d.%d", x->major, x->minor);
}

void tcl_version_init()
{
    if (tcl_version_class == 0) {
        tcl_version_class = class_new(gensym("_tcl.version"),
            (t_newmethod)tcl_version_new, 0,
            sizeof(t_tcl_version), 0, A_NULL);

        class_addmethod(tcl_version_class, (t_method)&tcl_version_set,
            gensym("tcl_version"), A_DEFFLOAT, A_NULL);
        pd_bind(&tcl_version_class, gensym("tcl_version"));
        tcl_version_instance = tcl_version_new();
        sys_gui("pdsend \"tcl_version tcl_version $tk_version\"\n");
    }
}

t_eclass* eclass_new(const char* name, t_typ_method newm, t_typ_method freem, size_t size, int flags, t_atomtype arg1, int arg2)
{
    t_class* pd = class_new(gensym(name), (t_newmethod)newm, (t_method)freem, size, flags, arg1, arg2);
    if (!pd) {
        bug("PureData Memory allocation failed for the class %s.", name);
        return 0;
    }

    // we are using same size to prevent PD from memset to 0 resized memory.
    // we should do this way because some versions of PD have different sizeof(t_class)
    t_eclass* c = (t_eclass*)resizebytes(pd, sizeof(t_eclass), sizeof(t_eclass));

    if (c) {
        tcl_version_init();
        epd_init();
        memset(&c->c_widget, 0, sizeof(t_ewidget));
        c->c_attr = 0;
        c->c_nattr = 0;
        c->c_dsp = 0;
        c->c_box = 0;
        c->c_attr = NULL;
    } else {
        bug("Memory allocation failed for the class %s.", name);
        return 0;
    }

    return c;
}

void eclass_init(t_eclass* c, long flags)
{
    eclass_guiinit(c, flags);
}

void eclass_guiinit(t_eclass* c, long flags)
{
    ewidget_init(c);
    c->c_box = 1;

    // DEFAULT ATTRIBUTES //
    CLASS_ATTR_FLOAT_ARRAY(c, "size", 0, t_ebox, b_rect.width, 2);
    CLASS_ATTR_SYMBOL(c, "fontname", 0, t_ebox, b_font.c_family);
    CLASS_ATTR_SYMBOL(c, "fontweight", 0, t_ebox, b_font.c_weight);
    CLASS_ATTR_SYMBOL(c, "fontslant", 0, t_ebox, b_font.c_slant);
    CLASS_ATTR_LONG(c, "fontsize", 0, t_ebox, b_font.c_sizereal);
    CLASS_ATTR_SYMBOL(c, "receive", 0, t_ebox, b_receive_id);
    CLASS_ATTR_SYMBOL(c, "send", 0, t_ebox, b_send_id);

    CLASS_ATTR_DEFAULT(c, "size", 0, "100. 100.");
    CLASS_ATTR_FILTER_MIN(c, "size", 4);
    CLASS_ATTR_SAVE(c, "size", 0);
    CLASS_ATTR_PAINT(c, "size", 0);
    CLASS_ATTR_CATEGORY(c, "size", 0, _("Basic"));
    CLASS_ATTR_LABEL(c, "size", 0, _("Patching Size"));
    CLASS_ATTR_ACCESSORS(c, "size", NULL, (t_err_method)ebox_size_set);

    CLASS_ATTR_INT(c, "pinned", 0, t_ebox, b_pinned);
    CLASS_ATTR_DEFAULT(c, "pinned", 0, "0");
    CLASS_ATTR_FILTER_CLIP(c, "pinned", 0, 1);
    CLASS_ATTR_SAVE(c, "pinned", 0);
    CLASS_ATTR_CATEGORY(c, "pinned", 0, _("Basic"));
    CLASS_ATTR_LABEL(c, "pinned", 0, _("Pinned"));
    CLASS_ATTR_STYLE(c, "pinned", 0, "onoff");

    CLASS_ATTR_DEFAULT(c, "fontname", 0, "Helvetica");
    CLASS_ATTR_SAVE(c, "fontname", 0);
    CLASS_ATTR_PAINT(c, "fontname", 0);
    CLASS_ATTR_CATEGORY(c, "fontname", 0, _("Basic"));
    CLASS_ATTR_LABEL(c, "fontname", 0, _("Font Name"));
    CLASS_ATTR_ACCESSORS(c, "fontname", NULL, (t_err_method)ebox_set_font);
    CLASS_ATTR_STYLE(c, "fontname", 0, "menu");
    CLASS_ATTR_ITEMS(c, "fontname", 0, "Helvetica Monaco Courier Times DejaVu");

    CLASS_ATTR_DEFAULT(c, "fontweight", 0, _("normal"));
    CLASS_ATTR_SAVE(c, "fontweight", 0);
    CLASS_ATTR_PAINT(c, "fontweight", 0);
    CLASS_ATTR_CATEGORY(c, "fontweight", 0, _("Basic"));
    CLASS_ATTR_LABEL(c, "fontweight", 0, _("Font Weight"));
    CLASS_ATTR_ACCESSORS(c, "fontweight", NULL, (t_err_method)ebox_set_fontweight);
    CLASS_ATTR_STYLE(c, "fontweight", 0, "menu");
    CLASS_ATTR_ITEMS(c, "fontweight", 0, "normal bold");

    CLASS_ATTR_DEFAULT(c, "fontslant", 0, "roman");
    CLASS_ATTR_SAVE(c, "fontslant", 0);
    CLASS_ATTR_PAINT(c, "fontslant", 0);
    CLASS_ATTR_CATEGORY(c, "fontslant", 0, _("Basic"));
    CLASS_ATTR_LABEL(c, "fontslant", 0, _("Font Slant"));
    CLASS_ATTR_ACCESSORS(c, "fontslant", NULL, (t_err_method)ebox_set_fontslant);
    CLASS_ATTR_STYLE(c, "fontslant", 0, "menu");
    CLASS_ATTR_ITEMS(c, "fontslant", 0, "roman italic");

    CLASS_ATTR_DEFAULT(c, "fontsize", 0, "11");
    CLASS_ATTR_FILTER_MIN(c, "fontsize", 4);
    CLASS_ATTR_SAVE(c, "fontsize", 0);
    CLASS_ATTR_PAINT(c, "fontsize", 0);
    CLASS_ATTR_CATEGORY(c, "fontsize", 0, _("Basic"));
    CLASS_ATTR_LABEL(c, "fontsize", 0, _("Font Size"));
    CLASS_ATTR_ACCESSORS(c, "fontsize", NULL, ebox_set_fontsize);
    CLASS_ATTR_STYLE(c, "fontsize", 0, "number");

    CLASS_ATTR_DEFAULT(c, "receive", 0, "(null)");
    CLASS_ATTR_ACCESSORS(c, "receive", NULL, ebox_set_receiveid);
    CLASS_ATTR_SAVE(c, "receive", 0);
    CLASS_ATTR_CATEGORY(c, "receive", 0, _("Basic"));
    CLASS_ATTR_LABEL(c, "receive", 0, _("Receive Symbol"));

    CLASS_ATTR_DEFAULT(c, "send", 0, "(null)");
    CLASS_ATTR_ACCESSORS(c, "send", NULL, ebox_set_sendid);
    CLASS_ATTR_SAVE(c, "send", 0);
    CLASS_ATTR_CATEGORY(c, "send", 0, _("Basic"));
    CLASS_ATTR_LABEL(c, "send", 0, _("Send Symbol"));

    // GUI always need this methods //
    class_addmethod((t_class*)c, (t_method)ebox_attr_dump, SYM_DUMP, A_NULL, 0);
    class_addmethod((t_class*)c, (t_method)ebox_output_all_attrs, SYM_GET_ALL_PROPS, A_NULL, 0);
    class_addmethod((t_class*)c, (t_method)ebox_dialog, SYM_DIALOG, A_GIMME, 0);

    class_addmethod((t_class*)c, (t_method)ebox_mouse_enter, SYM_MOUSE_ENTER, A_NULL, 0);
    class_addmethod((t_class*)c, (t_method)ebox_mouse_leave, SYM_MOUSE_LEAVE, A_NULL, 0);
    class_addmethod((t_class*)c, (t_method)ebox_mouse_move, SYM_MOUSE_MOVE, A_GIMME, 0);
    class_addmethod((t_class*)c, (t_method)ebox_mouse_down, SYM_MOUSE_DOWN, A_GIMME, 0);
    class_addmethod((t_class*)c, (t_method)ebox_mouse_up, SYM_MOUSE_UP, A_GIMME, 0);

    class_addmethod((t_class*)c, (t_method)ebox_pos, SYM_POS, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod((t_class*)c, (t_method)ebox_vis, SYM_VIS, A_DEFFLOAT, 0);
    class_addmethod((t_class*)c, (t_method)ebox_setzoom, SYM_ZOOM, A_DEFFLOAT, 0);

    class_setwidget((t_class*)&c->c_class, (t_widgetbehavior*)&c->c_widget);
    class_setsavefn((t_class*)&c->c_class, (t_savefn)eobj_save);
}

void eclass_dspinit(t_eclass* c)
{
    c->c_dsp = 1;
    class_addmethod((t_class*)c, (t_method)eobj_dsp, SYM_DSP, A_NULL, 0);
    class_addmethod((t_class*)c, (t_method)eobj_dsp_add, SYM_DSP_ADD, A_NULL, 0);
    class_addmethod((t_class*)c, (t_method)eobj_dsp_add, SYM_DSP_ADD64, A_NULL, 0);
}

static t_pd_err is_cicm(t_eobj* x)
{
    return 1;
}

t_pd_err eclass_register(t_symbol* name, t_eclass* c)
{
    if (c->c_dsp) {
        long diff = 0;

        if (c->c_box)
            diff = calcoffset(t_edspbox, d_dsp.d_float);
        else
            diff = calcoffset(t_edspobj, d_dsp.d_float);

        class_domainsignalin(&c->c_class, diff);
    }

    if (c->c_nattr) {
        eclass_properties_dialog(c);
        class_setpropertiesfn((t_class*)c, (t_propertiesfn)ebox_properties);
    }

    class_addmethod((t_class*)c, (t_method)is_cicm, s_iscicm, A_NULL, 0);

    return 0;
}

void eclass_addmethod(t_eclass* c, t_typ_method m, const char* name, t_atomtype type, long dummy)
{
    if (gensym(name) == SYM_MOUSE_ENTER) {
        c->c_widget.w_mouseenter = m;
    } else if (gensym(name) == SYM_MOUSE_LEAVE) {
        c->c_widget.w_mouseleave = m;
    } else if (gensym(name) == SYM_MOUSE_MOVE) {
        c->c_widget.w_mousemove = m;
    } else if (gensym(name) == SYM_MOUSE_DOWN) {
        c->c_widget.w_mousedown = m;
    } else if (gensym(name) == SYM_MOUSE_DRAG) {
        c->c_widget.w_mousedrag = m;
    } else if (gensym(name) == SYM_MOUSE_UP) {
        c->c_widget.w_mouseup = m;
    } else if (gensym(name) == SYM_MOUSE_WHEEL) {
        class_addmethod((t_class*)c, (t_method)ebox_mouse_wheel, SYM_MOUSE_WHEEL, A_GIMME, 0);
        c->c_widget.w_mousewheel = m;
    } else if (gensym(name) == SYM_MOUSE_DBL_CLICK) {
        class_addmethod((t_class*)c, (t_method)ebox_mouse_dblclick, SYM_MOUSE_DBL_CLICK, A_GIMME, 0);
        c->c_widget.w_dblclick = m;
    } else if (gensym(name) == SYM_KEY || gensym(name) == SYM_KEY_FILTER) {
        if (c->c_widget.w_key == NULL && c->c_widget.w_keyfilter == NULL)
            class_addmethod((t_class*)c, (t_method)ebox_key, SYM_KEY, A_GIMME, 0);
        if (gensym(name) == SYM_KEY)
            c->c_widget.w_key = m;
        if (gensym(name) == SYM_KEY_FILTER)
            c->c_widget.w_keyfilter = m;
    } else if (gensym(name) == SYM_PAINT) {
        c->c_widget.w_paint = m;
    } else if (gensym(name) == SYM_NOTIFY) {
        c->c_widget.w_notify = (t_err_method)m;
    } else if (gensym(name) == SYM_GET_DRAW_PARAMS) {
        c->c_widget.w_getdrawparameters = m;
    } else if (gensym(name) == SYM_OK_SIZE) {
        c->c_widget.w_oksize = m;
    } else if (gensym(name) == SYM_SAVE) {
        c->c_widget.w_save = m;
    } else if (gensym(name) == SYM_POPUP) {
        class_addmethod((t_class*)c, (t_method)eobj_popup, SYM_POPUP, A_SYMBOL, A_DEFFLOAT, 0);
        c->c_widget.w_popup = m;
    } else if (gensym(name) == SYM_DSP) {
        c->c_widget.w_dsp = m;
    } else if (gensym(name) == &s_bang) {
        class_addbang((t_class*)c, m);
    } else if (gensym(name) == &s_float) {
        class_addfloat((t_class*)c, m);
    } else if (gensym(name) == &s_list) {
        class_addlist((t_class*)c, m);
    } else if (gensym(name) == SYM_ANY) {
        class_addanything((t_class*)c, m);
    } else if (gensym(name) == &s_symbol) {
        class_addsymbol((t_class*)c, m);
    } else if (gensym(name) == SYM_PRESET) {
        CLASS_ATTR_SYMBOL(c, "presetname", 0, t_ebox, b_objpreset_id);
        CLASS_ATTR_DEFAULT(c, "presetname", 0, "(null)");
        CLASS_ATTR_SAVE(c, "presetname", 0);
        CLASS_ATTR_CATEGORY(c, "presetname", 0, _("Basic"));
        CLASS_ATTR_LABEL(c, "presetname", 0, _("Preset Name"));
        CLASS_ATTR_ACCESSORS(c, "presetname", NULL, ebox_set_presetid);
        class_addmethod((t_class*)c, (t_method)m, gensym(name), type, 0);
    } else {
        class_addmethod((t_class*)c, (t_method)m, gensym(name), type, 0);
    }
}

static t_symbol* gentype(const char* name)
{
    if (strcmp(name, "float") == 0)
        return &s_float;
    else if (strcmp(name, "symbol") == 0)
        return &s_symbol;
    else
        return gensym(name);
}

void eclass_new_attr_typed(t_eclass* c, const char* attrname, const char* type, long size, long maxsize, long flags, long offset)
{
    int i;
    t_eattr* attr;
    t_eattr** attrs;
    if (size >= 1) {
        t_symbol* name = gensym(attrname);

        for (i = 0; i < c->c_nattr; i++) {
            if (c->c_attr[i]->name == name) {
                error("%s already have %s attribute.", c->c_class.c_name->s_name, attrname);
                return;
            }
        }

        attr = (t_eattr*)getbytes(sizeof(t_eattr));

        if (attr) {
            attr->name = name;
            attr->type = gentype(type);
            attr->category = c->c_class.c_name;
            attr->label = gensym("");
            attr->style = SYM_ENTRY;
            attr->order = c->c_nattr + 1;
            attr->save = 0;
            attr->paint = 0;
            attr->invisible = 0;
            attr->flags = flags;
            attr->offset = offset;
            attr->size = size;
            attr->sizemax = maxsize;
            attr->getter = NULL;
            attr->setter = NULL;
            attr->clipped = 0;
            attr->minimum = 0;
            attr->maximum = 1;
            attr->step = 1;
            attr->defvals = NULL;
            attr->itemslist = NULL;
            attr->itemssize = 0;

            size_t new_sz = (size_t)(c->c_nattr + 1) * sizeof(t_eattr*);
            attrs = (t_eattr**)resizebytes(c->c_attr, new_sz, new_sz);

            if (attrs) {
                char buf[MAXPDSTRING];
                c->c_attr = attrs;
                c->c_attr[c->c_nattr] = attr;
                sprintf(buf, "@%s", attrname);
                class_addmethod((t_class*)c, (t_method)eclass_attr_ceammc_setter, gensym(buf), A_GIMME, 0);
                sprintf(buf, "@%s?", attrname);
                class_addmethod((t_class*)c, (t_method)eclass_attr_ceammc_getter, gensym(buf), A_GIMME, 0);
                c->c_nattr++;
            } else {
                error("%s can't increase memory for %s attribute.", c->c_class.c_name->s_name, attrname);
            }

        } else {
            error("%s can't allocate memory for %s attribute.", c->c_class.c_name->s_name, attrname);
        }
    } else {
        error("%s %s attribute size is too null.", c->c_class.c_name->s_name, attrname);
    }
}

void eclass_attr_redirect(t_eclass* c, const char* attrname, t_gotfn fn)
{
    char buf[MAXPDSTRING];
    sprintf(buf, "@%s", attrname);
    t_symbol* sel0 = gensym(buf);
    sprintf(buf, "@%s?", attrname);
    t_symbol* sel1 = gensym(buf);

    for (int i = 0; i < c->c_class.c_nmethod; i++) {
        t_methodentry* m = &c->c_class.c_methods[i];
        if (m->me_name == sel0 || m->me_name == sel1)
            m->me_fun = fn;
    }
}

void eclass_attr_default(t_eclass* c, const char* attrname, long flags, const char* value)
{
    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == gensym(attrname)) {
            c->c_attr[i]->defvals = gensym(value);
            return;
        }
    }
}

void eclass_attr_category(t_eclass* c, const char* attrname, long flags, const char* category)
{
    t_symbol* cat = gensym(category);
    t_symbol* sel = gensym(attrname);

    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == sel) {
            c->c_attr[i]->category = cat;

            if (cat == gensym(_("Basic")))
                c->c_attr[i]->order += CAT_BASE;
            else if (cat == gensym(_("Colors")))
                c->c_attr[i]->order += CAT_COLOR;
            else if (cat == gensym(_("Bounds")))
                c->c_attr[i]->order += CAT_BOUNDS;
            else if (cat == gensym(_("MIDI")))
                c->c_attr[i]->order += CAT_MIDI;
            else
                c->c_attr[i]->order += CAT_MISC;

            return;
        }
    }
}

void eclass_attr_order(t_eclass* c, const char* attrname, long flags, const char* order)
{
    t_symbol* sel = gensym(attrname);

    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == sel) {
            int n = atoi(order);
            if (isdigit(n))
                c->c_attr[i]->order = n;

            return;
        }
    }
}

void eclass_attr_label(t_eclass* c, const char* attrname, long flags, const char* label)
{
    t_symbol* sel = gensym(attrname);

    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == sel) {
            c->c_attr[i]->label = gensym(label);
            return;
        }
    }
}

void eclass_attr_style(t_eclass* c, const char* attrname, long flags, const char* style)
{
    t_symbol* sel = gensym(attrname);
    t_symbol* s_style = gensym(style);

    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == sel) {
            if (s_style == SYM_CHECKBUTTON || s_style == SYM_ON_OFF) {
                c->c_attr[i]->style = SYM_CHECKBUTTON;
            } else if (s_style == SYM_RGB || s_style == SYM_RGBA || s_style == SYM_COLOR) {
                c->c_attr[i]->style = SYM_COLOR;
            } else if (s_style == SYM_SPINBOX || s_style == SYM_NUMBER) {
                c->c_attr[i]->style = SYM_NUMBER;
            } else if (s_style == SYM_MENU) {
                c->c_attr[i]->style = SYM_MENU;
            } else if (s_style == SYM_PATH) {
                c->c_attr[i]->style = SYM_PATH;
            } else {
                c->c_attr[i]->style = SYM_ENTRY;
            }
            return;
        }
    }
}

void eclass_attr_itemlist(t_eclass* c, const char* attrname, long flags, const char* list)
{
    int i, j = 0;
    char* pch;
    int size = 0;
    t_symbol* s_attrname = gensym(attrname);
    for (i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == s_attrname) {
            pch = strtok(gensym(list)->s_name, " ,");

            while (pch != NULL) {
                pch = strtok(NULL, " ,");
                size++;
            }
            if (size > 0) {
                if (c->c_attr[i]->itemssize) {
                    size_t new_sz = (unsigned long)size * sizeof(t_symbol*);
                    c->c_attr[i]->itemslist = (t_symbol**)resizebytes(c->c_attr[i]->itemslist, new_sz, new_sz);
                    if (c->c_attr[i]->itemslist)
                        c->c_attr[i]->itemssize = size;
                } else {
                    c->c_attr[i]->itemslist = (t_symbol**)getbytes((unsigned long)size * sizeof(t_symbol*));
                    if (c->c_attr[i]->itemslist)
                        c->c_attr[i]->itemssize = size;
                }
                if (c->c_attr[i]->itemslist && c->c_attr[i]->itemssize) {
                    pch = strtok(gensym(list)->s_name, " ,");
                    while (pch != NULL && (long)j < c->c_attr[i]->itemssize) {
                        c->c_attr[i]->itemslist[j] = gensym(pch);
                        pch = strtok(NULL, " ,");
                        j++;
                    }
                }

            } else {
                if (c->c_attr[i]->itemssize)
                    freebytes(c->c_attr[i]->itemslist, 0);
                c->c_attr[i]->itemssize = 0;
            }
            return;
        }
    }
}

void eclass_attr_filter_min(t_eclass* c, const char* attrname, float value)
{
    t_symbol* s_attrname = gensym(attrname);
    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == s_attrname) {
            if (c->c_attr[i]->clipped == 0)
                c->c_attr[i]->clipped = 1;
            else if (c->c_attr[i]->clipped == 2)
                c->c_attr[i]->clipped = 3;

            c->c_attr[i]->minimum = value;
            return;
        }
    }
}

void eclass_attr_filter_max(t_eclass* c, const char* attrname, float value)
{
    t_symbol* s_attrname = gensym(attrname);
    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == s_attrname) {
            if (c->c_attr[i]->clipped == 0)
                c->c_attr[i]->clipped = 2;
            else if (c->c_attr[i]->clipped == 1)
                c->c_attr[i]->clipped = 3;

            c->c_attr[i]->maximum = value;
            return;
        }
    }
}

void eclass_attr_step(t_eclass* c, const char* attrname, float value)
{
    t_symbol* s_attrname = gensym(attrname);
    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == s_attrname) {
            c->c_attr[i]->step = value;
            return;
        }
    }
}

void eclass_attr_save(t_eclass* c, const char* attrname, long flags)
{
    t_symbol* s_attrname = gensym(attrname);
    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == s_attrname) {
            c->c_attr[i]->save = 1;
            return;
        }
    }
}

void eclass_attr_paint(t_eclass* c, const char* attrname, long flags)
{
    t_symbol* s_attrname = gensym(attrname);
    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == s_attrname) {
            c->c_attr[i]->paint = 1;
            return;
        }
    }
}

void eclass_attr_invisible(t_eclass* c, const char* attrname, long flags)
{
    t_symbol* s_attrname = gensym(attrname);
    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == s_attrname) {
            c->c_attr[i]->invisible = 1;
            return;
        }
    }
}

void eclass_attr_accessor(t_eclass* c, const char* attrname, t_err_method getter, t_err_method setter)
{
    t_symbol* s_attrname = gensym(attrname);
    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == s_attrname) {
            c->c_attr[i]->getter = getter;
            c->c_attr[i]->setter = setter;
            return;
        }
    }
}

void eclass_attr_getter(t_object* x, t_symbol* s, int* argc, t_atom** argv)
{
    t_ebox* z = (t_ebox*)x;
    t_eclass* c = (t_eclass*)z->b_obj.o_obj.te_g.g_pd;

    if (argc == NULL) {
        pd_error(x, "[%s] null argc pointer given", class_getname(x->te_pd));
        return;
    }

    if (*argv) {
        pd_error(x, "[%s] invalid argv pointer given", class_getname(x->te_pd));
        return;
    }

    *argc = 0;

    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name != s)
            continue;

        char* point = (char*)x + c->c_attr[i]->offset;

        if (c->c_attr[i]->sizemax == 0) {
            *argc = (int)c->c_attr[i]->size;
        } else {
            point = (char*)x + c->c_attr[i]->size;
            *argc = (int)point[0];
            if (*argc > c->c_attr[i]->sizemax)
                *argc = (int)c->c_attr[i]->sizemax;
        }

        t_symbol* type = c->c_attr[i]->type;

        argv[0] = (t_atom*)getbytes((size_t)*argc * sizeof(t_atom));

        if (c->c_attr[i]->getter) {
            c->c_attr[i]->getter(x, c->c_attr[i], argc, argv);
        } else if (type == s_int) {
            for (int j = 0; j < *argc; j++) {
                atom_setlong(argv[0] + j, ((int*)point)[j]);
            }
        } else if (type == s_long) {
            for (int j = 0; j < *argc; j++) {
                atom_setlong(argv[0] + j, ((long*)point)[j]);
            }
        } else if (type == &s_float) {
            for (int j = 0; j < *argc; j++) {
                atom_setfloat(argv[0] + j, ((float*)point)[j]);
            }
        } else if (type == s_double) {
            for (int j = 0; j < *argc; j++) {
                atom_setfloat(argv[0] + j, (float)(((double*)point)[j]));
            }
        } else if (type == &s_symbol) {
            t_symbol** syms = (t_symbol**)point;
            for (int j = 0; j < *argc; j++) {
                if (syms[j]) {
                    atom_setsym(argv[0] + j, gensym(syms[j]->s_name));
                }
            }
        } else if (type == s_atom) {
            for (int j = 0; j < *argc; j++) {
                argv[0][j] = ((t_atom*)point)[j];
            }
        } else {
            printf("Unknown property get method: %s\n", type->s_name);
        }

        break;
    }
}

void eclass_attr_ceammc_getter(t_object* x, t_symbol* s, int a, t_atom* l)
{
    int argc_ = 0;
    t_atom* argv_ = NULL;
    t_ebox* z = (t_ebox*)x;
    if (!z->b_obj.o_obj.te_outlet) {
        pd_error(x, "[%s] can't get property: class has no outlets.", class_getname(x->te_pd));
        return;
    }

    const size_t len = strlen(s->s_name);
    if (len < 3 || len > MAXPDSTRING) {
        pd_error(x, "[%s] invalid property name", class_getname(x->te_pd));
        return;
    }

    char buf[MAXPDSTRING];
    // copy property name without leading '@' char and ending '?' char
    memcpy(buf, s->s_name + 1, len - 2);
    buf[len - 2] = '\0';
    t_symbol* prop_name = gensym(buf);
    // NOTE: argv points to allocated array
    eclass_attr_getter(x, prop_name, &argc_, &argv_);

    if (argc_ > 0) {
        // copy property name without ending '?' char
        memcpy(buf, s->s_name, len - 1);
        buf[len - 1] = '\0';
        t_symbol* prop_at_name = gensym(buf);
        outlet_anything(z->b_obj.o_obj.te_outlet, prop_at_name, argc_, argv_);
        // free memory allocated in eclass_attr_getter()
        freebytes(argv_, 0);
    } else {
        memcpy(buf, s->s_name, len - 1);
        buf[len - 1] = '\0';
        pd_error(x, "[%s] unknown property: %s", class_getname(x->te_pd), buf);
    }
}

static void eclass_attr_ceammc_setter(t_object* x, t_symbol* s, int argc, t_atom* argv)
{
    const size_t len = strlen(s->s_name);
    if (len < 2 || len > MAXPDSTRING) {
        pd_error(x, "[%s] invalid property name", class_getname(x->te_pd));
        return;
    }

    char buf[MAXPDSTRING];
    // copy property name without leading '@' char
    memcpy(buf, s->s_name + 1, len);
    buf[len] = '\0';
    t_symbol* prop_name = gensym(buf);
    eclass_attr_setter(x, prop_name, argc, argv);
}

void eclass_attr_setter(t_object* x, t_symbol* s, int argc, t_atom* argv)
{
    long i, j, size;
    char* point;
    long* point_size;
    t_ebox* z = (t_ebox*)x;
    t_eclass* c = (t_eclass*)z->b_obj.o_obj.te_g.g_pd;

    for (i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == s) {
            t_symbol* type = c->c_attr[i]->type;
            if (c->c_attr[i]->sizemax == 0) {
                size = c->c_attr[i]->size;
            } else {
                if (argc > c->c_attr[i]->sizemax) {
                    argc = (int)c->c_attr[i]->sizemax;
                }
                size = argc;
                point = (char*)x + c->c_attr[i]->size;
                point_size = (long*)point;
                point_size[0] = (long)size;
            }

            point = (char*)(x) + c->c_attr[i]->offset;

            if (c->c_attr[i]->clipped == 1 || c->c_attr[i]->clipped == 3) {
                for (j = 0; j < argc; j++) {
                    if (atom_gettype(argv + j) == A_FLOAT) {
                        atom_setfloat(argv + j, (float)pd_clip_min(atom_getfloat(argv + j), c->c_attr[i]->minimum));
                    }
                }
            }
            if (c->c_attr[i]->clipped == 2 || c->c_attr[i]->clipped == 3) {
                for (j = 0; j < argc; j++) {
                    if (atom_gettype(argv + j) == A_FLOAT) {
                        atom_setfloat(argv + j, (float)pd_clip_max(atom_getfloat(argv + j), c->c_attr[i]->maximum));
                    }
                }
            }

            if (c->c_attr[i]->setter) {
                c->c_attr[i]->setter(x, c->c_attr[i], argc, argv);
            } else if (type == s_int) {
                int* pointor = (int*)point;
                for (j = 0; j < size && j < argc; j++) {
                    if (atom_gettype(argv + j) == A_FLOAT) {
                        pointor[j] = (int)atom_getlong(argv + j);
                    }
                }
            } else if (type == s_long) {
                long* pointor = (long*)point;
                for (j = 0; j < size && j < argc; j++) {
                    if (atom_gettype(argv + j) == A_FLOAT) {
                        pointor[j] = (long)atom_getlong(argv + j);
                    }
                }
            } else if (type == &s_float) {
                float* pointor = (float*)point;
                for (j = 0; j < size && j < argc; j++) {
                    if (atom_gettype(argv + j) == A_FLOAT) {
                        pointor[j] = (float)atom_getfloat(argv + j);
                    }
                }
            } else if (type == s_double) {
                double* pointor = (double*)point;
                for (j = 0; j < size && j < argc; j++) {
                    if (atom_gettype(argv + j) == A_FLOAT) {
                        pointor[j] = (double)atom_getfloat(argv + j);
                    }
                }
            } else if (type == &s_symbol) {
                t_symbol** pointor = (t_symbol**)point;
                for (j = 0; j < size && j < argc; j++) {
                    if (atom_gettype(argv + j) == A_SYMBOL) {
                        pointor[j] = gensym(atom_getsymbol(argv + j)->s_name);
                    }
                }
            } else if (type == s_atom) {
                t_atom* pointor = (t_atom*)point;
                for (j = 0; j < size && j < argc; j++) {
                    pointor[j] = argv[j];
                }
            }

            ebox_notify(z, s, s_attr_modified, NULL, NULL);
            if (c->c_widget.w_notify != NULL) {
                c->c_widget.w_notify(x, s, s_attr_modified, NULL, NULL);
            }

            if (c->c_attr[i]->paint) {
                if (c->c_widget.w_oksize != NULL) {
                    c->c_widget.w_oksize(x, &z->b_rect);
                }
                if (c->c_widget.w_getdrawparameters != NULL) {
                    c->c_widget.w_getdrawparameters(x, NULL, &z->b_boxparameters);
                }

                ebox_redraw(z);
            }
            if (c->c_attr[i]->save && eobj_isbox(x) && ebox_isdrawable((t_ebox*)x)) {
                canvas_dirty(eobj_getcanvas(x), 1);
            }
        }
    }
}

static void ewidget_init(t_eclass* c)
{
    c->c_widget.w_getrectfn = ebox_wgetrect;
    c->c_widget.w_visfn = ebox_wvis;
    c->c_widget.w_displacefn = ebox_wdisplace;
    c->c_widget.w_selectfn = ebox_wselect;
    c->c_widget.w_activatefn = NULL;
    c->c_widget.w_deletefn = ebox_wdelete;
    c->c_widget.w_clickfn = NULL;

    c->c_widget.w_paint = NULL;
    c->c_widget.w_mouseenter = NULL;
    c->c_widget.w_mouseleave = NULL;
    c->c_widget.w_mousemove = NULL;
    c->c_widget.w_mousedown = NULL;
    c->c_widget.w_mousedrag = NULL;
    c->c_widget.w_mouseup = NULL;
    c->c_widget.w_mousewheel = NULL;
    c->c_widget.w_dblclick = NULL;
    c->c_widget.w_key = NULL;
    c->c_widget.w_keyfilter = NULL;
    c->c_widget.w_getdrawparameters = NULL;
    c->c_widget.w_notify = NULL;
    c->c_widget.w_save = NULL;
    c->c_widget.w_dosave = NULL;
    c->c_widget.w_popup = NULL;
    c->c_widget.w_dsp = NULL;
    c->c_widget.w_oksize = NULL;
    c->c_widget.w_write = NULL;
    c->c_widget.w_read = NULL;
}

#define DIALOG_GRID_PADY " -pady 1 "
#define DIALOG_WINDOW_PADX " -padx 8 "
#define DIALOG_WINDOW_PADY " -pady 8 "

#ifdef __APPLE__
#define DIALOG_BACKGROUND " -background #ECECEC "
#else
#define DIALOG_BACKGROUND " -background #ECECEC "
#endif

static const char* dialog_label_frame(int i)
{
    static char buf[100];
    snprintf(buf, 100, "$id.top_frame.name%i", i);
    return buf;
}

static const char* dialog_widget_frame(int i)
{
    static char buf[100];
    snprintf(buf, 100, "$id.top_frame.sele%i", i);
    return buf;
}

static const char* dialog_label_id(int i)
{
    static char buf[100];
    snprintf(buf, 100, "$id.top_frame.name%i.name", i);
    return buf;
}

static const char* dialog_widget_id(int i)
{
    static char buf[100];
    snprintf(buf, 100, "$id.top_frame.sele%i.selec", i);
    return buf;
}

//! Initialize the tcl/tk properties dialog window functions // PRIVATE
/*
 \ @memberof            eclass
 \ @param c             The eclass pointer
 \ @return              Nothing
 */
static void eclass_properties_dialog(t_eclass* c)
{
    int i, j, lenght;
    char buffer[1000];
    char temp[1000];

    // DIALOG WINDOW APPLY //
    for (i = 0; i < c->c_nattr; i++) {
        const char* ATTR_NAME = c->c_attr[i]->name->s_name;

        if (c->c_attr[i]->style == SYM_COLOR) {
            sys_vgui("proc pdtk_%s_picker_apply_%s {id red green blue alpha} { \n", c->c_class.c_name->s_name, ATTR_NAME);
            sys_gui("set nR [expr int( $red * 65025 )]\n");
            sys_gui("set nG [expr int( $green * 65025 )]\n");
            sys_gui("set nB [expr int( $blue * 65025 )]\n");
            sys_gui("set col [format {%4.4x} $nR]\n");
            sys_gui("append col [format {%4.4x} $nG]\n");
            sys_gui("append col [format {%4.4x} $nB]\n");
            sys_vgui("wm attributes $id -topmost 0 \n");
            sys_vgui("set color [tk_chooseColor -title {%s} -initialcolor #${col} -parent $id ]\n", c->c_attr[i]->label->s_name, ATTR_NAME);
            sys_vgui("wm attributes $id -topmost 1 \n");
            sys_gui("if {$color == \"\"} return \n");
            sys_gui("foreach {red2 green2 blue2} [winfo rgb . $color] {}\n");
            sys_gui("set nR2 [expr ( $red2 / 65025. )]\n");
            sys_gui("set nG2 [expr ( $green2 / 65025. )]\n");
            sys_gui("set nB2 [expr ( $blue2 / 65025. )]\n");
            sys_gui("if {$nR2 > 1.} {set nR2 1.} \n");
            sys_gui("if {$nG2 > 1.} {set nG2 1.} \n");
            sys_gui("if {$nB2 > 1.} {set nB2 1.} \n");
            sprintf(buffer, "set cmd [concat $id dialog $id %i ", i + 1);
            sprintf(temp, "@%s ", ATTR_NAME);
            lenght = (int)strlen(temp);
            strncat(buffer, temp, lenght);
            sprintf(temp, "[concat $nR2 $nG2 $nB2] ");
            lenght = (int)strlen(temp);
            strncat(buffer, temp, lenght);
            strncat(buffer, "]\n", 2);
            sys_gui(buffer);
            sys_gui("pdsend $cmd\n");
            sys_gui("}\n");
        } else {
            sys_vgui("proc pdtk_%s_dialog_apply_%s {id} { \n", c->c_class.c_name->s_name, ATTR_NAME);
            sys_gui("set vid [string trimleft $id .]\n");
            sys_vgui("set var_%s [concat %s_$vid] \n", ATTR_NAME, ATTR_NAME);
            sys_vgui("global $var_%s \n", ATTR_NAME);
            sprintf(buffer, "set cmd [concat $id dialog $id %i ", i + 1);
            sprintf(temp, "@%s ", ATTR_NAME);
            lenght = (int)strlen(temp);
            strncat(buffer, temp, lenght);
            sprintf(temp, "[eval concat $$var_%s] ", ATTR_NAME);
            lenght = (int)strlen(temp);
            strncat(buffer, temp, lenght);
            strncat(buffer, "]\n", 2);
            sys_gui(buffer);
            sys_gui("pdsend $cmd\n");
            sys_gui("}\n");
        }
    }

    // DIALOG WINDOW CREATION //
    sys_vgui("proc pdtk_%s_dialog {id \n", c->c_class.c_name->s_name);
    for (i = 0; i < c->c_nattr; i++) {
        const char* ATTR_NAME = c->c_attr[i]->name->s_name;
        if (!c->c_attr[i]->invisible) {
            sys_vgui("%s \n", ATTR_NAME);
        }
    }
    sys_gui("} {\n");
    sys_gui("set vid [string trimleft $id .]\n");

    for (i = 0; i < c->c_nattr; i++) {
        const char* ATTR_NAME = c->c_attr[i]->name->s_name;
        if (!c->c_attr[i]->invisible) {
            sys_vgui("set var_%s [string trim [concat %s_$vid]]\n", ATTR_NAME, ATTR_NAME);
            sys_vgui("global $var_%s \n", ATTR_NAME);
            sys_vgui("set $var_%s [string trim $%s]\n", ATTR_NAME, ATTR_NAME);
        }
    }
    sys_vgui("toplevel $id\n");
    sys_vgui("wm title $id {%s properties} \n", c->c_class.c_name->s_name);
    sys_vgui("wm resizable $id 0 0\n");
    sys_vgui("raise [winfo toplevel $id]\n");

    //CEAMMC
    sys_vgui("$id configure " DIALOG_BACKGROUND DIALOG_WINDOW_PADX DIALOG_WINDOW_PADY "\n");
    sys_vgui("ttk::frame $id.top_frame\n");
    sys_vgui("grid $id.top_frame\n");

    for (i = 0; i < c->c_nattr; i++) {
        if (!c->c_attr[i]->invisible) {
            const char* LABEL_FRAME = dialog_label_frame(i + 1);
            const char* WIDGET_FRAME = dialog_widget_frame(i + 1);
            const char* LABEL_ID = dialog_label_id(i + 1);
            const char* WIDGET_ID = dialog_widget_id(i + 1);
            const char* ATTR_NAME = c->c_attr[i]->name->s_name;
            const char* CLASS_NAME = c->c_class.c_name->s_name;

            sys_vgui("frame %s\n", LABEL_FRAME);
            sys_vgui("frame %s\n", WIDGET_FRAME);

            /** ATTRIBUTES NAMES **/

            sys_vgui("ttk::label %s -justify left -text [join [list [_ \"%s\" ] {:}] {}]\n", LABEL_ID, c->c_attr[i]->label->s_name);
            sys_vgui("pack %s -side left\n", LABEL_ID);

            /** SELECTOR WIDGETS **/

            if (c->c_attr[i]->style == SYM_CHECKBUTTON) {
                sys_vgui("ttk::checkbutton %s -variable [string trim $var_%s] "
                         "-command  [concat pdtk_%s_dialog_apply_%s $id]\n",
                    WIDGET_ID, ATTR_NAME, CLASS_NAME, ATTR_NAME);
                sys_vgui("pack %s -side left\n", WIDGET_ID);
            } else if (c->c_attr[i]->style == SYM_COLOR) {
                sys_vgui("set color [eval eobj_rgba_to_hex $%s]\n", ATTR_NAME);
                sys_vgui("entry %s -font {Helvetica 11} -width 10 -readonlybackground $color -state readonly\n", WIDGET_ID);
                sys_vgui("bind  %s <Button> [concat pdtk_%s_picker_apply_%s $id $%s]\n", WIDGET_ID, CLASS_NAME, ATTR_NAME, ATTR_NAME);
                sys_vgui("pack %s -side left\n", WIDGET_ID);
            } else if (c->c_attr[i]->style == SYM_NUMBER) {
                sys_vgui("ttk::spinbox %s -width 18 -textvariable [string trim $var_%s] -increment %f \n", WIDGET_ID, ATTR_NAME, (float)c->c_attr[i]->step);
                sys_vgui("%s configure -command [concat pdtk_%s_dialog_apply_%s $id]\n", WIDGET_ID, CLASS_NAME, ATTR_NAME);
                sys_vgui("%s configure -from -9999999999999 -to 9999999999999\n", WIDGET_ID, (float)c->c_attr[i]->maximum); // Should be enough
                sys_vgui("%s delete 0 end \n", WIDGET_ID);
                sys_vgui("%s insert 0 $%s \n", WIDGET_ID, ATTR_NAME);

                sys_vgui("bind %s <KeyPress-Return> [concat pdtk_%s_dialog_apply_%s $id]\n", WIDGET_ID, CLASS_NAME, ATTR_NAME);
                sys_vgui("pack %s -side left\n", WIDGET_ID);
            } else if (c->c_attr[i]->style == SYM_MENU) {
                sys_vgui("ttk::combobox %s -width 16 -state readonly -textvariable [string trim $var_%s]\n", WIDGET_ID, ATTR_NAME);
                sys_vgui("%s configure -values { ", WIDGET_ID);
                for (int j = 0; j < c->c_attr[i]->itemssize; j++) {
                    sys_vgui("%s ", c->c_attr[i]->itemslist[c->c_attr[i]->itemssize - 1 - j]->s_name);
                }
                sys_vgui("}\n");

                sys_vgui("bind %s <<ComboboxSelected>> [concat pdtk_%s_dialog_apply_%s $id]\n", WIDGET_ID, CLASS_NAME, ATTR_NAME);
                sys_vgui("pack %s -side left\n", WIDGET_ID);
                sys_vgui("%s set [string trim $%s] \n", WIDGET_ID, ATTR_NAME);
            } else if (c->c_attr[i]->style == SYM_PATH) {
                sys_vgui("ttk::entry %s -width 20 -textvariable [string trim $var_%s]\n", WIDGET_ID, ATTR_NAME);
                //                sys_vgui("bind %s <FocusIn> { if { [string trim [%%W get]] == {(null)} } { %%W delete 0 end } }\n", WIDGET_ID);
                //                sys_vgui("bind %s <FocusOut> { if { [string trim [%%W get]] == {} } { %%W insert 0 {(null)} } }\n", WIDGET_ID);
                sys_vgui("bind %s <KeyPress-Return> [concat pdtk_%s_dialog_apply_%s $id]\n", WIDGET_ID, CLASS_NAME, ATTR_NAME);
                sys_vgui("pack %s -side left\n", WIDGET_ID);

                sys_vgui("proc cicm_dialog_%s_open_%s {varname id} {\n", CLASS_NAME, ATTR_NAME);
                sys_vgui("global $varname\n");
                sys_vgui("puts $varname\n");
                sys_vgui("set types {\n");
                sys_vgui("{{Image Files}        {.gif .png .jpg .jpeg}}\n");
                sys_vgui("}\n");
                sys_vgui("set $varname [tk_getOpenFile -initialfile $$varname -filetypes $types]\n");
                sys_vgui("pdtk_%s_dialog_apply_%s $id\n", CLASS_NAME, ATTR_NAME);
                sys_vgui("}\n");

                char btn_id[120];
                snprintf(btn_id, 120, "%s_%s_file", WIDGET_ID, ATTR_NAME);
                sys_vgui("ttk::button %s -text %s -command [concat cicm_dialog_%s_open_%s [string trim $var_%s] $id]\n",
                    btn_id,
                    _("Select"),
                    CLASS_NAME,
                    ATTR_NAME,
                    ATTR_NAME);
                sys_vgui("pack %s -side left\n", btn_id);
            } else {
                sys_vgui("ttk::entry %s -width 20 -textvariable [string trim $var_%s]\n", WIDGET_ID, ATTR_NAME);
                sys_vgui("bind %s <FocusIn> { if { [string trim [%%W get]] == {(null)} } { %%W delete 0 end } }\n", WIDGET_ID);
                sys_vgui("bind %s <FocusOut> { if { [string trim [%%W get]] == {} } { %%W insert 0 {(null)} } }\n", WIDGET_ID);
                sys_vgui("bind %s <KeyPress-Return> [concat pdtk_%s_dialog_apply_%s $id]\n", WIDGET_ID, CLASS_NAME, ATTR_NAME);
                sys_vgui("pack %s -side left\n", WIDGET_ID);
            }

            sys_vgui("grid config %s -column 0 -row %i -sticky w" DIALOG_GRID_PADY "\n", LABEL_FRAME, i + 1);
            sys_vgui("grid config %s -column 1 -row %i -sticky w" DIALOG_GRID_PADY "\n", WIDGET_FRAME, i + 1);
        }
    }
    sys_gui("}\n");
}

static int attr_cmp(const void* p1, const void* p2)
{
    const t_eattr* a1 = *(const t_eattr**)p1;
    const t_eattr* a2 = *(const t_eattr**)p2;

    if (a1->order == a2->order)
        return 0;
    else if (a1->order < a2->order)
        return -1;
    else
        return 1;
}

void eclass_attr_sort(t_eclass* c)
{
    qsort(c->c_attr, c->c_nattr, sizeof(t_eattr*), attr_cmp);
}

void eclass_attr_visible(t_eclass* c, const char* attrname, long flags)
{
    t_symbol* s_attrname = gensym(attrname);
    for (int i = 0; i < c->c_nattr; i++) {
        if (c->c_attr[i]->name == s_attrname) {
            c->c_attr[i]->invisible = 0;
            return;
        }
    }
}
