//
//  list_interleave.cpp
//  pd_ext
//
//  Created by Alex Nadzharov on 23/12/16.
//

#include "ceammc.hpp"
#include "ceammc_factory.h"
#include "ceammc_fn_list.h"
#include "ceammc_log.h"
#include "ceammc_object.h"
#include "list_interleave.h"
#include "ceammc_format.h"

#include <cassert>
#include <sstream>

using namespace ceammc;

static const size_t MIN_INLET = 2;
static const size_t MAX_INLET = 20;

extern "C" void setup_list0x2einterleave()
{
    ObjectFactory<ListInterleave> obj("list.interleave");
}

ListInterleave::ListInterleave(const PdArgs &a)
    : BaseObject(a)
    , in_count_(math::clip(atomlistToValue<size_t>(a.args, MIN_INLET), MIN_INLET, MAX_INLET))
    , method_(0)
    , pad_(0.f)
    , gmin_(0)
    , gpad_(0)
    , gclip_(0)
    , gwrap_(0)
    , gfold_(0)
{
    initLists();
    initInlets();
    createOutlet();
    initProperties();
}

void ListInterleave::onBang()
{
    listTo(0, out_list_);
}

void ListInterleave::onList(const AtomList &l)
{
    in_list_[0] = l;
    const t_symbol* m = method_->value();

    if (m == gmin_)
        out_list_ = list::interleaveMinLength(in_list_);
    else if (m == gpad_)
        out_list_ = list::interleavePadWith(in_list_, pad_);
    else if (m == gclip_)
        out_list_ = list::interleaveClip(in_list_);
    else if (m == gwrap_)
        out_list_ = list::interleaveWrap(in_list_);
    else if (m == gfold_)
        out_list_ = list::interleaveFold(in_list_);

    onBang();
}

void ListInterleave::onInlet(size_t n, const AtomList &l)
{
    if (n >= in_count_) {
        OBJ_ERR << "invalid inlet index: " << n << ", total inlet count = " << in_count_;
        return;
    }

    in_list_[n] = l;
}

void ListInterleave::dump() const
{
    BaseObject::dump();

    OBJ_DBG << "input lists:";
    for (size_t i = 0; i < in_list_.size(); i++) {
        OBJ_DBG << in_list_[i];
    }

    OBJ_DBG << "out list: " << out_list_;
}

void ListInterleave::initInlets()
{
    assert(in_count_ > 1);

    for (size_t i = 0; i < in_count_ - 1; i++)
        createInlet();
}

void ListInterleave::initLists()
{
    in_list_.reserve(in_count_);

    for (size_t i = 0; i < in_count_; i++)
        in_list_.push_back(AtomList());


    for(size_t i = 0; i < in_count_; i++) {
        std::ostringstream s;
        s << "@l" << i;
        PointerProperty<AtomList>* p = new PointerProperty<AtomList>(s.str().c_str(), &in_list_[i], true);
        createProperty(p);
    }
}

void ListInterleave::initProperties()
{
    // interleave methods:
    // @min - interleave by shortests list (by default)
    // @pad - pad with specified value (@pad_value property), zero by default
    // @wrap - pad with wrapped values
    // @fold - pad with fold values
    method_ = new SymbolEnumProperty("@method", "min");
    method_->appendEnum("pad");
    method_->appendEnum("clip");
    method_->appendEnum("wrap");
    method_->appendEnum("fold");
    createProperty(method_);

    // init t_symbol* pointer members for fast compare
    gmin_ = gensym("min");
    gpad_ = gensym("pad");
    gclip_ = gensym("clip");
    gwrap_ = gensym("wrap");
    gfold_ = gensym("fold");

    // adding aliases
    createProperty(new SymbolEnumAlias("@min", method_, gmin_));
    createProperty(new SymbolEnumAlias("@clip", method_, gclip_));
    createProperty(new SymbolEnumAlias("@wrap", method_, gwrap_));
    createProperty(new SymbolEnumAlias("@fold", method_, gfold_));

    createCbProperty("@pad", &ListInterleave::getPadValue, &ListInterleave::setPadValue);
}

AtomList ListInterleave::getPadValue() const { return listFrom(pad_); }

void ListInterleave::setPadValue(const AtomList &l)
{
    pad_ = atomlistToValue<Atom>(l, Atom(0.f));
    method_->setValue(gpad_);
}
