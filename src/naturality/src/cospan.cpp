#include "naturality/cospan.hpp"

#include <algorithm>
#include <functional>

namespace {
using namespace Project::Naturality;
using namespace Project::Types;

CospanStructure::MappedType create_default(TypeConstructor::AtomicType const &);
CospanStructure create_default(TypeConstructor::ConstructorType const &);

struct CreateDefaultMappedType {

  CospanStructure::MappedType operator()(Variance variance,
                                         std::size_t identifier) const {
    return CospanStructure::MappedType{0, variance};
  }

  CospanStructure::MappedType
  operator()(Variance variance, FunctorTypeConstructor const &functor) const {
    return CospanStructure::MappedType{create_default(functor.type), variance};
  }

  CospanStructure::MappedType
  operator()(Variance variance, TypeConstructor const &type_constructor) const {
    return CospanStructure::MappedType{create_default(type_constructor.type),
                                       variance};
  }

  template <typename T>
  CospanStructure::MappedType operator()(Variance variance, T const &) const {
    return CospanStructure::MappedType{EmptyType(), variance};
  }

} _create_default_mapped_type;

CospanStructure::MappedType
create_default_type(TypeConstructor::AtomicType const &type) {
  auto const create_cospan_type = std::bind(
      _create_default_mapped_type, type.variance, std::placeholders::_1);
  return std::visit(create_cospan_type, type.type);
}

std::vector<CospanStructure::MappedType>
create_default(TypeConstructor::ConstructorType const &constructor) {
  std::vector<CospanStructure::MappedType> default_values;
  default_values.reserve(type_constructor.size());
  std::transform(constructor.begin(), constructor.end(),
                 std::back_inserter(default_values), create_default_type);
  return std::move(default_values);
}

std::vector<CospanStructure::MappedType>
create_default(TypeConstructor const &constructor) {
  return create_defalut(constructor.type);
}

} // namespace

namespace Project {
namespace Naturality {

CospanStructure create_default_cospan(Types::TypeConstructor const &domain,
                                      Types::TypeConstructor const &codomain) {
  return CospanStructure{create_default(domain), create_default(codomain)};
}

} // namespace Naturality
} // namespace Project