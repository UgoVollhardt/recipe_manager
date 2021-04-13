#include "c_component.h"
#include "c_ingredient.h"

c_component::c_component(const c_ingredient &_ingredient, const int _quantity, const int _unit, const int _id) : quantity(_quantity), unit(_unit), id(_id) {
    ingredient = new c_ingredient;
    *ingredient = _ingredient;
}

c_component::c_component(const c_component &other) {
    ingredient = new c_ingredient;
    *ingredient = other.getIngredient();
    quantity = other.getQuantity();
    unit = other.getUnit();
    id = other.getId();
}

c_component::~c_component() {
    if (ingredient != nullptr) {
        free(ingredient);
    }
}

c_ingredient& c_component::getIngredient() {
    return *ingredient;
}

c_ingredient& c_component::getIngredient() const{
    return *ingredient;
}

void c_component::setIngredient(const c_ingredient &value) {
    *ingredient = value;
}

int c_component::getQuantity() const {
    return quantity;
}

void c_component::setQuantity(int value) {
    quantity = value;
}

int c_component::getUnit() const {
    return unit;
}

void c_component::setUnit(int value) {
    unit = recipe::unitToString.keys().contains(value)?value:recipe::unit::none;
}

int c_component::getId() const {
    return id;
}

void c_component::setId(int value) {
    id = value;
}

c_component &c_component::operator=(const c_component &other) {
    if (ingredient == nullptr) {
        ingredient = new c_ingredient;
    }
    *ingredient = other.getIngredient();
    id = other.getId();
    quantity = other.getQuantity();
    unit = other.getUnit();
    return *this;
}

bool c_component::operator==(const c_component &other) const {
    return (other.getIngredient() == *ingredient)
            && (other.getQuantity() == quantity)
            && (other.getUnit() == unit);
}
