#ifndef TRANSLATEDFIELD_H
#define TRANSLATEDFIELD_H

#include "Concepts.h"

template<DifferentiableField FIELD> class TranslatedField;


template<FirstOrderField FIELD>
class TranslatedField<FIELD> : public FIELD {
public:
    typedef FIELD::SpaceTime SpaceTime;

    const SpaceTime origin;

    TranslatedField(const TranslatedField<FIELD> & field) : FIELD(field) {};
    TranslatedField(TranslatedField<FIELD> && field) : FIELD(std::move(field)) {};
    TranslatedField(TranslatedField<FIELD> field, SpaceTime translation) : FIELD(std::move(field)), origin(field.origin + translation) { }
    TranslatedField(SpaceTime translation) : FIELD(), origin(std::move(translation)) {}
    TranslatedField(FIELD field, SpaceTime translation) : FIELD(std::move(field)), origin(std::move(translation)) {}

    auto operator ()(const SpaceTime &pos) const { return FIELD::operator ()(pos-origin); }
    
};


template<SecondOrderField FIELD>
class TranslatedField<FIELD> : public FIELD {
public:
    typedef FIELD::SpaceTime SpaceTime;

    const SpaceTime origin;

    TranslatedField(const TranslatedField<FIELD> & field) : FIELD(field) {};
    TranslatedField(TranslatedField<FIELD> && field) : FIELD(std::move(field)) {};
    TranslatedField(TranslatedField<FIELD> field, SpaceTime translation) : FIELD(std::move(field)), origin(field.origin + translation) { }
    TranslatedField(SpaceTime translation) : FIELD(), origin(std::move(translation)) {}
    TranslatedField(FIELD field, SpaceTime translation) : FIELD(std::move(field)), origin(std::move(translation)) {}

    auto operator ()(const SpaceTime &pos) const { return FIELD::operator ()(pos-origin); }
    
    auto d_dt(const SpaceTime &pos, const SpaceTime &vel) const {
        return FIELD::d_dt(pos-origin, vel); 
    }
};

template<HigherOrderField FIELD>
class TranslatedField<FIELD> : public FIELD {
public:
    typedef FIELD::SpaceTime SpaceTime;

    const SpaceTime origin;

    TranslatedField() = default;
    TranslatedField(const TranslatedField<FIELD> & field) : FIELD(field) {};
    TranslatedField(TranslatedField<FIELD> && field) : FIELD(std::move(field)) {};
    TranslatedField(TranslatedField<FIELD> field, SpaceTime translation) : FIELD(std::move(field)), origin(field.origin + translation) { }
    TranslatedField(SpaceTime translation) : FIELD(), origin(std::move(translation)) {}
    TranslatedField(FIELD field, SpaceTime translation) : FIELD(std::move(field)), origin(std::move(translation)) {}

    auto operator ()(const SpaceTime &pos) const { return FIELD::operator ()(pos-origin); }
    
    auto d_dt(const SpaceTime &pos, const SpaceTime &vel) const {
        return FIELD::d_dt(pos-origin, vel); 
    }

    auto d2_dt2(const SpaceTime &pos, const SpaceTime &vel) const {
        return FIELD::d2_dt2(pos-origin, vel); 
    }
};

template<DifferentiableField FIELD>
TranslatedField(FIELD field, typename FIELD::SpaceTime translation) -> TranslatedField<FIELD>;
template<DifferentiableField FIELD>
TranslatedField(TranslatedField<FIELD> field, typename FIELD::SpaceTime translation) -> TranslatedField<FIELD>;

// template<class FIELD>
// class TranslatedField<TranslatedField<FIELD>> : public TranslatedField<FIELD> {
//     TranslatedField(TranslatedField<FIELD> field, SpaceTime translation) : TranslatedField<FIELD>(std::move(field), field.origin + translation) {
//     }
    
//     TranslatedField(SpaceTime translation) : TranslatedField<FIELD>(std::move(translation)) {}

// }


#endif
