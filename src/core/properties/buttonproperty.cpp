#include <inviwo/core/properties/buttonproperty.h>

namespace inviwo {
    ButtonProperty::ButtonProperty(std::string identifier, std::string displayName,PropertySemantics::Type semantics /*= PropertySemantics::Default*/)
	: Property(identifier,displayName,semantics)
	{}


void ButtonProperty::serialize(inviwo::IvwSerializer &s) const {
	Property::serialize(s);
}
	
void ButtonProperty::deserialize(IvwDeserializer& d) {}

} //namespace
