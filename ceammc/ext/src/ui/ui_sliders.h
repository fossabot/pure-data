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
#ifndef UI_SLIDERS_H
#define UI_SLIDERS_H

#include "ceammc_ui_object.h"

using namespace ceammc;

class UISliders : public UIObject {
    UIFont txt_font_;
    UITextLayout txt_min_;
    UITextLayout txt_max_;
    long select_idx_;
    bool is_vertical_;
    std::vector<float> pos_values_;
    UILayer sliders_layer_;

private:
    t_rgba prop_slider_color;
    t_rgba prop_select_color;
    float prop_min;
    float prop_max;
    int prop_auto_range;
    int prop_show_range;
    int prop_count;
    char c_min[16];
    char c_max[16];

public:
    UISliders();

    void okSize(t_rect* newrect);
    t_pd_err notify(t_symbol* attrname, t_symbol* msg);
    void paint(t_object* view);
    void paintBackground();
    void paintSliders();

    void onBang();
    void onList(const AtomList& lst);
    AtomList realValues() const;

    // presets
    void loadPreset(size_t idx);
    void storePreset(size_t idx);

    // mouse
    void onMouseDown(t_object* view, const t_pt& pt, long modifiers);
    void onMouseDrag(t_object* view, const t_pt& pt, long modifiers);
    void onDblClick(t_object* view, const t_pt& pt, long modifiers);

    // methods
    void m_get(const AtomList& l);
    void m_set(const AtomList& l);
    void m_select(const AtomList& l);
    void m_plus(t_float f);
    void m_minus(t_float f);
    void m_mul(t_float f);
    void m_div(t_float f);
    void m_increment();
    void m_decrement();
    void m_fill(t_float v);

    void outputList();
    void normalize();

private:
    void setRealValueAt(size_t n, t_float v);
    float realValueAt(size_t n) const;
    bool setRealValues(const AtomList& l);

    float propCount() const;
    void setPropCount(float f);
    float propAutoRange() const;
    void setPropAutoRange(float f);

    float propRange() const;
    AtomList propValue() const;

    void generateTxtLabels();

public:
    static void setup();

private:
    void redrawSliders();
    void redrawAll();
};

void setup_ui_sliders();

#endif // UI_SLIDERS_H
