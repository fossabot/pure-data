#include "ui_env.h"
#include "ceammc_datatypes.h"
#include "ceammc_preset.h"
#include "ceammc_ui.h"

#include <boost/range.hpp>
#include <limits>

static const int ENV_MIN_WIDTH = 40;
static const int ENV_MIN_HEIGHT = 30;
static t_symbol* GRID_COLOR = gensym("#C0C0C0");
static t_symbol* ENV_COLOR = gensym("#505050");
static t_symbol* ENV_COLOR2 = gensym("#00C0FF");
static t_symbol* ENV_ACTIVE = gensym("#FF0000");
static t_rgba DELETE_COLOR = hex_to_rgba("#F00030");
static const char* DEFAULT_LINE_COLOR = "0.1 0.1 0.1 1.0";
static const char* PROP_LENGTH = "length";

static t_symbol* SYM_ADSR = gensym("adsr");
static t_symbol* SYM_ASR = gensym("asr");
static t_symbol* SYM_AR = gensym("ar");
static t_symbol* SYM_EADSR = gensym("eadsr");
static t_symbol* SYM_EASR = gensym("easr");
static t_symbol* SYM_EAR = gensym("ear");
static t_symbol* SYM_LENGTH = gensym("length");

static float lin2lin(float v, float x0, float x1, float y0, float y1)
{
    return convert::lin2lin(clip(v, x0, x1), x0, x1, y0, y1);
}

UIEnv::UIEnv()
    : draw_cursor_pos_(false)
    , draw_cursor_cross_(false)
    , delete_mode_(false)
    , cursor_layer_(asEBox(), gensym("cursor_layer"))
    , font_(FONT_FAMILY, FONT_SIZE_SMALL)
    , cursor_txt_pos_(font_.font(), ColorRGBA::black(), ETEXT_UP, ETEXT_JCENTER)
    , max_env_value_(1)
    , txt_font(FONT_FAMILY, FONT_SIZE_SMALL)
    , txt_value0(txt_font.font(), ColorRGBA::black(), ETEXT_UP_LEFT, ETEXT_JLEFT)
    , txt_value1(txt_font.font(), ColorRGBA::black(), ETEXT_DOWN_LEFT, ETEXT_JLEFT)
    , txt_value2(txt_font.font(), ColorRGBA::black(), ETEXT_DOWN_LEFT, ETEXT_JLEFT)
{
    cursor_pos_.x = 0;
    cursor_pos_.y = 0;
    cursor_txt_pos_.setWidth(100);
    cursor_txt_pos_.setY(FONT_SIZE_SMALL);

    createOutlet();
    env_.setADSR(40 * 1000, 60 * 1000, 0.3, 400 * 1000);
    updateNodes();
}

void UIEnv::onBang()
{
    dataTo(0, DataPtr(env_.clone()));
}

void UIEnv::onData(const DataPtr& ptr)
{
    if (ptr->type() != data::DATA_ENVELOPE) {
        UI_ERR << "not Envelope received: " << ptr->toString();
        return;
    }

    if (ptr.isNull()) {
        UI_ERR << "NULL pointer";
        return;
    }

    const DataTypeEnv* env = ptr->as<DataTypeEnv>();

    if (env->numPoints() < 2) {
        UI_ERR << "invalid (for editor) envelope : " << *env;
        return;
    }

    env_ = env->normalize();
    updateNodes();
    redrawAll();
    onBang();
}

void UIEnv::paint(t_object*)
{
    const t_rect& r = rect();
    UIPainter bp = bg_layer_.painter(r);

    if (bp) {
        const size_t total = nodes_.size();

        // draw grid
        // horizontal
        bp.setColor(rgba_addContrast(prop_color_background, 0.05));

        bp.drawLine(-1, height() * 0.25, width(), height() * 0.25);
        bp.drawLine(-1, height() * 0.5, width(), height() * 0.5);
        bp.drawLine(-1, height() * 0.75, width(), height() * 0.75);

        txt_value0.setColor(prop_color_border);
        txt_value0.setPos(3, 3);
        txt_value0.set("1.0");
        bp.drawText(txt_value0);

        txt_value1.setColor(prop_color_border);
        txt_value1.setPos(3, height() * 0.5 - 2);
        txt_value1.set("0.5");
        bp.drawText(txt_value1);

        txt_value2.setColor(prop_color_border);
        txt_value2.setPos(3, height() - 2);
        txt_value2.set("0.0");
        bp.drawText(txt_value2);

        // vertical
        float len_ms = env_.totalLength() / 1000;
        float p = log10f(len_ms);
        float np = 0;
        if (modff(p, &np) < 0.35)
            np -= 1;

        float step = ((powf(10, np)) / len_ms) * width();
        if (step > 3) {
            for (float x = 0; x < width(); x += step) {
                bp.drawLine(x, 0, x, height());
            }
        }

        for (size_t i = 0; i < total; i++) {
            const Node& n = nodes_[i];

            bp.setColor(prop_line_color);

            // draw segments
            if (i != (total - 1)) { // skip last point
                const Node& next = nodes_[i + 1];

                switch (n.type) {
                case CURVE_LINE:
                    bp.drawLine(n.x, n.y, next.x, next.y);
                    break;
                case CURVE_STEP:
                    bp.drawLine(n.x, n.y, next.x, n.y);
                    bp.drawLine(next.x, n.y, next.x, next.y);
                    break;
                case CURVE_EXP: {

                    bp.moveTo(n.x, n.y);

                    int i = n.x;
                    for (; i < next.x; i += 4) {
                        float y = convert::lin2curve(float(i), n.x, next.x, n.y, next.y, n.curve);
                        bp.drawLineTo(i, y);
                    }

                    bp.drawLineTo(next.x, next.y);
                    bp.stroke();

                } break;
                case CURVE_SIN2: {

                    bp.moveTo(n.x, n.y);

                    int i = n.x;
                    for (; i < next.x; i += 4) {
                        float y = convert::lin2sin2(i, n.x, next.x, n.y, next.y);
                        bp.drawLineTo(i, y);
                    }

                    bp.drawLineTo(next.x, next.y);
                    bp.stroke();

                } break;
                case CURVE_SIGMOID: {

                    bp.moveTo(n.x, n.y);

                    int i = n.x;
                    for (; i < next.x; i += 4) {
                        float y = convert::lin2sigmoid(i, n.x, next.x, n.y, next.y, n.sigmoid_skew);
                        bp.drawLineTo(i, y);
                    }

                    bp.drawLineTo(next.x, next.y);
                    bp.stroke();

                } break;
                default:
                    break;
                }
            }

            // draw selection rectangle
            if (n.is_selected) {
                if (delete_mode_)
                    bp.setColor(DELETE_COLOR);

                bp.drawRect(n.x - 6, n.y - 6, 12, 12);
                bp.stroke();
            }

            // draw point itself
            bp.setColor(prop_line_color);
            bp.drawRect(n.x - 3, n.y - 3, 6, 6);
            bp.fill();

            // draw vertical dash line for stop point
            if (n.is_stop) {
                bp.setDashStyle(EDASHSTYLE_24);
                bp.drawLine(n.x, 0, n.x, height());
                bp.setDashStyle(EDASHSTYLE_NONE);
            }
        }
    }

    UIPainter cp = cursor_layer_.painter(r);

    if (cp) {

        // draw add point cross
        if (draw_cursor_cross_) {
            cp.setColor(prop_active_color);
            cp.drawLine(0, cursor_pos_.y, width(), cursor_pos_.y);
            cp.drawLine(cursor_pos_.x, 0, cursor_pos_.x, height());
        }

        if (draw_cursor_pos_) {
            char buf[100];

            snprintf(buf, sizeof(buf) - 1, "%d(ms) : %0.2f",
                (int)lin2lin(cursor_pos_.x, 0, width(), 0, prop_length),
                lin2lin(cursor_pos_.y, 0, height(), max_env_value_, 0));

            cursor_txt_pos_.setColor(prop_color_border);
            cursor_txt_pos_.set(buf, r.width / 2, 3, r.width, 20);
            cp.drawText(cursor_txt_pos_);
        }
    }
}

void UIEnv::okSize(t_rect* newrect)
{
    newrect->width = pd_clip_min(newrect->width, ENV_MIN_WIDTH);
    newrect->height = pd_clip_min(newrect->height, ENV_MIN_HEIGHT);

    updateNodes();
}

void UIEnv::onMouseMove(t_object*, const t_pt& pt, long modifiers)
{
    int idx = findNearestNode(pt.x, pt.y);

    if (selectNode(idx < 0 ? nodes_.size() : idx))
        bg_layer_.invalidate();

    // draw cursor position
    if (modifiers == EMOD_SHIFT) {
        draw_cursor_pos_ = true;
        draw_cursor_cross_ = true;
        cursor_pos_ = pt;
    } else if (modifiers == EMOD_ALT) {
        delete_mode_ = true;
        bg_layer_.invalidate();
    } else {
        delete_mode_ = false;
        draw_cursor_cross_ = false;
        draw_cursor_pos_ = (idx >= 0);

        if (idx >= 0) {
            cursor_pos_.x = nodes_[idx].x;
            cursor_pos_.y = nodes_[idx].y;
        }
    }

    redrawCursorLayer();
}

void UIEnv::onMouseDrag(t_object*, const t_pt& pt, long)
{
    long idx = -1;
    for (size_t i = 0; i < nodes_.size(); i++) {
        if (nodes_[i].is_selected) {
            idx = i;
            break;
        }
    }

    // no selected node
    if (idx < 0)
        return;

    Node& n = nodes_[idx];

    // update coordinates
    if (!n.fixed_x)
        n.x = clip<float>(pt.x, 0, width());

    if (!n.fixed_y)
        n.y = clip<float>(pt.y, 0, height());

    // x-coord
    if (nodes_.size() > 1 && idx > 0 && idx < (nodes_.size() - 1)) {
        Node& prev = nodes_[idx - 1];
        Node& next = nodes_[idx + 1];

        if (prev.x > n.x)
            std::swap(n, prev);
        else if (next.x < n.x)
            std::swap(n, next);
    }

    draw_cursor_cross_ = false;
    draw_cursor_pos_ = true;
    cursor_pos_ = pt;

    redrawAll();
}

void UIEnv::onMouseDown(t_object*, const t_pt& pt, long mod)
{
    if (mod & EMOD_SHIFT) {

        long insert_idx = 0;
        for (size_t i = 1; i < nodes_.size(); i++) {
            // prevent to close insertion
            if (fabsf(nodes_[i].x - pt.x) < 2)
                return;

            // find node index to insert before
            if (nodes_[i].x > pt.x) {
                insert_idx = i;
                break;
            }
        }

        Node n;
        n.x = pt.x;
        n.y = pt.y;
        n.is_selected = true;

        // insert new selected node
        nodes_.insert(nodes_.begin() + insert_idx, n);
        redrawBGLayer();
        return;
    }

    // find selected node index
    long idx = -1;
    for (size_t i = 0; i < nodes_.size(); i++) {
        if (nodes_[i].is_selected) {
            idx = i;
            break;
        }
    }

    // no selected node
    if (idx < 0)
        return;

    if (mod & EMOD_CTRL) {
        nodes_[idx].is_stop = !nodes_[idx].is_stop;
        redrawBGLayer();
        return;
    }

    if (mod & EMOD_ALT) {
        long n = nodes_.size();
        if (idx < 1 || idx >= n - 1)
            return;

        nodes_.erase(nodes_.begin() + idx);
        delete_mode_ = false;
        redrawBGLayer();
        return;
    }
}

void UIEnv::onMouseLeave(t_object*, const t_pt& pt, long)
{
    draw_cursor_cross_ = false;
    draw_cursor_pos_ = false;
    redrawCursorLayer();
}

void UIEnv::onMouseWheel(t_object*, const t_pt& pt, long, double delta)
{
    long idx = -1;
    for (size_t i = 1; i < nodes_.size(); i++) {
        if (pt.x < nodes_[i].x) {
            idx = i - 1;
            break;
        }
    }

    if (idx < 0)
        return;

    switch (nodes_[idx].type) {
    case CURVE_EXP:
        nodes_[idx].curve += (delta < 0) ? -0.2 : 0.2;
        break;
    case CURVE_SIGMOID:
        nodes_[idx].sigmoid_skew += (delta < 0) ? -0.2 : 0.2;
        break;
    default:
        return;
    }

    redrawBGLayer();
}

void UIEnv::onMouseUp(t_object*, const t_pt& pt, long)
{
    updateEnvelope();
    dataTo(0, DataPtr(env_.clone()));
}

void UIEnv::onDblClick(t_object*, const t_pt& pt, long modifiers)
{
    if (!(modifiers & EMOD_CTRL))
        return;

    long idx = -1;
    for (size_t i = 1; i < nodes_.size(); i++) {
        if (pt.x < nodes_[i].x) {
            idx = i - 1;
            break;
        }
    }

    if (idx < 0)
        return;

    Node& n = nodes_[idx];
    CurveType types[] = { CURVE_STEP, CURVE_LINE, CURVE_EXP, CURVE_SIN2, CURVE_SIGMOID };
    for (size_t i = 0; i < boost::size(types); i++) {
        if (n.type == types[i]) {
            n.type = types[(i + 1) % boost::size(types)];
            break;
        }
    }

    redrawBGLayer();
}

void UIEnv::updateNodes()
{
    nodes_.clear();

    size_t total_us = env_.totalLength();
    size_t n = env_.numPoints();

    for (size_t i = 0; i < n; i++) {
        bool is_edge = (i == 0) || (i == (n - 1));

        nodes_.push_back(Node::fromEnvelope(env_.pointAt(i), total_us, width(), height(), is_edge));
    }

    prop_length = total_us / 1000.0;
}

void UIEnv::updateEnvelope()
{
    env_.clear();

    for (size_t i = 0; i < nodes_.size(); i++) {
        const Node& n = nodes_[i];

        EnvelopePoint pt(n.x / width() * prop_length * 1000,
            1 - (n.y / height()),
            n.is_stop,
            n.type,
            n.curve);

        pt.sigmoid_skew = n.sigmoid_skew;

        env_.insertPoint(pt);
    }
}

int UIEnv::findNearestNode(float x, float y) const
{
    float min_dist = std::numeric_limits<float>::max();
    int min_idx = -1;

    // find minimal distance
    for (size_t i = 0; i < nodes_.size(); i++) {
        const Node& n = nodes_[i];
        float dist = std::min(min_dist, (n.x - x) * (n.x - x) + (n.y - y) * (n.y - y));

        if (dist < min_dist) {
            min_dist = dist;
            min_idx = i;
        }
    }

    if (min_dist < 36)
        return min_idx;

    return -1;
}

bool UIEnv::selectNode(size_t idx)
{
    int num_changes = 0;

    for (size_t i = 0; i < nodes_.size(); i++) {
        bool v = (i == idx);

        if (v != nodes_[i].is_selected) {
            nodes_[i].is_selected = v;
            num_changes++;
        }
    }

    return num_changes > 0;
}

void UIEnv::m_adsr(const AtomList& lst)
{
    setNamedEnvelope(SYM_ADSR, lst);
}

void UIEnv::m_asr(const AtomList& lst)
{
    setNamedEnvelope(SYM_ASR, lst);
}

void UIEnv::m_ar(const AtomList& lst)
{
    setNamedEnvelope(SYM_AR, lst);
}

void UIEnv::m_eadsr(const AtomList& lst)
{
    setNamedEnvelope(SYM_EADSR, lst);
}

void UIEnv::m_easr(const AtomList& lst)
{
    setNamedEnvelope(SYM_EASR, lst);
}

void UIEnv::m_ear(const AtomList& lst)
{
    setNamedEnvelope(SYM_EAR, lst);
}

void UIEnv::setNamedEnvelope(t_symbol* env, const AtomList& args)
{
    if (!env_.setNamedEnvelope(env, args)) {
        UI_ERR << "unknown envelope: " << Atom(env) + args;
        return;
    }

    updateNodes();

    redrawAll();
}

void UIEnv::loadPreset(size_t idx)
{
    AtomList lst = PresetStorage::instance().listValueAt(presetId(), idx);
    DataTypeEnv env = DataTypeEnv::fromList(lst);
    if (env.empty())
        return;

    env_ = env;
    updateNodes();
    redrawAll();
}

void UIEnv::storePreset(size_t idx)
{
    PresetStorage::instance().setListValueAt(presetId(), idx, env_.toList());
}

t_pd_err UIEnv::notify(t_symbol* attr_name, t_symbol* msg)
{
    if (attr_name == SYM_LENGTH) {
        if (env_.totalLength() != prop_length * 1000) {
            env_.resizeTime(prop_length * 1000);
            updateNodes();
            redrawAll();
        }
    }

    return UIObject::notify(attr_name, msg);
}

void UIEnv::redrawCursorLayer()
{
    cursor_layer_.invalidate();
    redraw();
}

void UIEnv::redrawAll()
{
    bg_layer_.invalidate();
    cursor_layer_.invalidate();
    redraw();
}

void UIEnv::setup()
{
    UIObjectFactory<UIEnv> obj("ui.env");
    obj.setDefaultSize(200, 150);

    obj.usePresets();
    obj.useBang();
    obj.useMouseEvents(UI_MOUSE_DOWN | UI_MOUSE_DRAG
        | UI_MOUSE_MOVE | UI_MOUSE_LEAVE
        | UI_MOUSE_WHEEL | UI_MOUSE_UP | UI_MOUSE_DBL_CLICK);
    obj.useData();

    obj.addProperty(PROP_ACTIVE_COLOR, _("Active Color"), DEFAULT_ACTIVE_COLOR, &UIEnv::prop_active_color);
    obj.addProperty("line_color", _("Line Color"), DEFAULT_LINE_COLOR, &UIEnv::prop_line_color);
    obj.addProperty(PROP_LENGTH, _("Length"), 400, &UIEnv::prop_length);
    obj.setPropertyMin(PROP_LENGTH, 10);

    obj.addMethod(SYM_ADSR, &UIEnv::m_adsr);
    obj.addMethod(SYM_ASR, &UIEnv::m_asr);
    obj.addMethod(SYM_AR, &UIEnv::m_ar);
    obj.addMethod(SYM_EADSR, &UIEnv::m_eadsr);
    obj.addMethod(SYM_EASR, &UIEnv::m_easr);
    obj.addMethod(SYM_EAR, &UIEnv::m_ear);
}

void setup_ui_env()
{
    UIEnv::setup();
}

Node Node::fromEnvelope(const EnvelopePoint& pt, size_t total_us, float w, float h, bool fixed_x)
{
    Node n;

    n.x = (pt.utime * w) / total_us;
    n.y = h * (1 - pt.value);
    n.curve = pt.data;
    n.sigmoid_skew = pt.sigmoid_skew;
    n.type = pt.type;
    n.is_selected = false;
    n.is_stop = pt.stop;
    n.fixed_x = fixed_x;
    n.fixed_y = false;

    return n;
}
