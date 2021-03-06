/*****************************************************************************
 * Copyright 2018 Serge Poltavsky. All rights reserved.
 *
 * This file may be distributed under the terms of GNU Public License version
 * 3 (GPL v3) as defined by the Free Software Foundation (FSF). A copy of the
 * license should have been included with this file, or the project in which
 * this file belongs to. You may also find the details of GPL v3 at:
 * http://www.gnu.org/licenses/gpl-3.0.txt
 *
 * If you have any questions regarding the use of this file, feel free to
 * contact the author of this file, or the owner of the project in which
 * this file belongs to.
 *****************************************************************************/
#include "ui_gain.h"
#include "ceammc_convert.h"
#include "ceammc_dsp_ui.h"
#include "ceammc_preset.h"

static const float SCALE_ALPHA_BLEND = 0.7;
static t_symbol* SYM_PROP_DB = gensym("@db");

UIGain::UIGain()
    : prop_color_knob(rgba_blue)
    , prop_color_scale(rgba_blue)
    , prop_max(0)
    , prop_min(-60)
    , prop_output_value(0)
    , font_(FONT_FAMILY, FONT_SIZE_SMALL)
    , txt_max_(font_.font(), ColorRGBA::black(), ETEXT_UP_LEFT, ETEXT_JLEFT)
    , txt_min_(font_.font(), ColorRGBA::black(), ETEXT_DOWN_LEFT, ETEXT_JLEFT)
    , knob_pos_(0)
    , is_horizontal_(false)
{
}

void UIGain::okSize(t_rect* newrect)
{
    newrect->width = pd_clip_min(newrect->width, 8.);
    newrect->height = pd_clip_min(newrect->height, 8.);

    is_horizontal_ = (newrect->width > newrect->height);

    if (is_horizontal_)
        newrect->width = pd_clip_min(newrect->width, 50.);
    else
        newrect->height = pd_clip_min(newrect->height, 50.);
}

void UIGain::paint(t_object* view)
{
    const t_rect& r = rect();
    UIPainter p = bg_layer_.painter(r);

    if (!p)
        return;

    txt_max_.setColor(prop_color_border);
    txt_min_.setColor(prop_color_border);
    p.setLineWidth(3);

    if (is_horizontal_) {
        float x = r.width * knob_pos_;
        // scale
        p.setColor(prop_color_scale);
        p.drawRect(0, 0, x, r.height);
        p.fill();

        // knob
        p.setColor(prop_color_knob);
        p.drawLine(x, 0, x, r.height);

        // levels
        p.drawText(txt_max_);
        p.drawText(txt_min_);

    } else {
        float y = r.height * (1 - knob_pos_);
        // scale
        p.setColor(prop_color_scale);
        p.drawRect(0, y, r.width, r.height - y);
        p.fill();

        // knob
        p.setColor(prop_color_knob);
        p.drawLine(0, y, r.width, y);

        // levels
        p.drawText(txt_max_);
        p.drawText(txt_min_);
    }
}

void UIGain::init(t_symbol* name, const AtomList& args, bool usePresets)
{
    UIDspObject::init(name, args, usePresets);
    dspSetup(1, 1);
}

void UIGain::dspProcess(t_sample** ins, long n_ins, t_sample** outs, long n_outs, long sampleframes)
{
    t_sample* in = ins[0];
    t_sample* out = outs[0];

    const float v = ampValue();

    for (long i = 0; i < sampleframes; i++) {
        out[i] = in[i] * smooth_.get(v);
    }
}

t_pd_err UIGain::notify(t_symbol* attr_name, t_symbol* msg)
{
    if (msg == s_attr_modified) {
        // update text labels
        updateLabels();

        // recalc scale color
        prop_color_scale = rgba_color_sum(&prop_color_background, &prop_color_knob, SCALE_ALPHA_BLEND);

        // redraw
        bg_layer_.invalidate();
        redraw();
    }

    return 0;
}

void UIGain::onBang()
{
    AtomList v(dbValue());
    anyTo(0, SYM_PROP_DB, v);
    send(SYM_PROP_DB, v);
}

void UIGain::onMouseDown(t_object* view, const t_pt& pt, long modifiers)
{
    onMouseDrag(view, pt, modifiers);
}

void UIGain::onMouseDrag(t_object* view, const t_pt& pt, long modifiers)
{
    if (is_horizontal_)
        knob_pos_ = clip<float>(pt.x / width(), 0, 1);
    else
        knob_pos_ = clip<float>(1 - pt.y / height(), 0, 1);

    redrawBGLayer();

    if (prop_output_value)
        onBang();
}

void UIGain::onDblClick(t_object* view, const t_pt& pt, long modifiers)
{
    t_canvas* c = reinterpret_cast<t_canvas*>(view);
    if (c->gl_edit) {
        resize(height(), width());
        updateLabels();
        redrawBGLayer();
    }
}

t_float UIGain::dbValue() const
{
    return convert::lin2lin<t_float>(knob_pos_, 1, 0, prop_max, prop_min);
}

t_float UIGain::ampValue() const
{
    t_float db = dbValue();
    if (db <= prop_min)
        return 0;

    return convert::dbfs2amp(db);
}

void UIGain::setDbValue(t_float db)
{
    knob_pos_ = clip<t_float>(convert::lin2lin<t_float>(db, prop_max, prop_min, 1, 0), 0, 1);
    redrawBGLayer();
}

void UIGain::setAmpValue(t_float amp)
{
    setDbValue(convert::amp2dbfs(amp));
}

void UIGain::loadPreset(size_t idx)
{
    setDbValue(PresetStorage::instance().floatValueAt(presetId(), idx, -60));
}

void UIGain::storePreset(size_t idx)
{
    PresetStorage::instance().setFloatValueAt(presetId(), idx, dbValue());
}

void UIGain::m_plus(t_float db)
{
    setDbValue(dbValue() + db);
}

void UIGain::m_minus(t_float db)
{
    m_plus(-db);
}

void UIGain::m_inc()
{
    m_plus(1);
}

void UIGain::m_dec()
{
    m_minus(1);
}

void UIGain::setup()
{
    UIDspFactory<UIGain> obj("ui.gain~");

    obj.addProperty("knob_color", _("Knob Color"), DEFAULT_ACTIVE_COLOR, &UIGain::prop_color_knob);
    obj.addProperty("db", &UIGain::dbValue, &UIGain::setDbValue);
    obj.addProperty("amp", &UIGain::ampValue, &UIGain::setAmpValue);
    obj.addIntProperty("max", _("Maximum value"), 0, &UIGain::prop_max);
    obj.addIntProperty("min", _("Minimum value"), -60, &UIGain::prop_min);
    obj.setPropertyRange("max", -6, 6);
    obj.setPropertyRange("min", -90, -50);
    obj.addBoolProperty("output_value", _("Output value"), false, &UIGain::prop_output_value);

    obj.setDefaultSize(15, 120);
    obj.usePresets();
    obj.useBang();

    obj.useMouseEvents(UI_MOUSE_DOWN | UI_MOUSE_DRAG | UI_MOUSE_DBL_CLICK);
    obj.addMethod("+", &UIGain::m_plus);
    obj.addMethod("-", &UIGain::m_minus);
    obj.addMethod("++", &UIGain::m_inc);
    obj.addMethod("--", &UIGain::m_dec);
    obj.addMethod("set", &UIGain::setDbValue);
}

void UIGain::updateLabels()
{
    char buf[32];

    // MAX
    snprintf(buf, 5, "%d", prop_max);
    if (is_horizontal_) {
        txt_max_.setAnchor(ETEXT_RIGHT);
        txt_max_.setJustify(ETEXT_JRIGHT);
        txt_max_.set(buf, width() - 2, height() / 2, 40, 20);
    } else {
        txt_max_.setAnchor(ETEXT_UP_LEFT);
        txt_max_.setJustify(ETEXT_JLEFT);
        txt_max_.set(buf, 2, 2, 40, 20);
    }

    // MIN
    snprintf(buf, 5, "%d", prop_min);
    if (is_horizontal_) {
        txt_min_.setAnchor(ETEXT_LEFT);
        txt_min_.setJustify(ETEXT_JLEFT);
        txt_min_.set(buf, 2, height() / 2, 40, 20);
    } else {
        txt_min_.setAnchor(ETEXT_DOWN_LEFT);
        txt_min_.setJustify(ETEXT_JLEFT);
        txt_min_.set(buf, 2, height() - 2, 40, 20);
    }
}

void setup_ui_gain()
{
    UIGain::setup();
}
