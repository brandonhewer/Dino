#include "naturality/cospan.hpp"
#include "naturality/natural_transformation.hpp"

#include <algorithm>
#include <functional>

namespace {
using namespace Project::Naturality;
using namespace Project::Types;

CospanMorphism::MappedType
create_default_type(TypeConstructor::AtomicType const &);

CospanMorphism create_default(TypeConstructor::ConstructorType const &);

struct CreateDefaultMappedType {

  CospanMorphism::MappedType operator()(Variance variance,
                                        std::size_t identifier) const {
    return CospanMorphism::MappedType{0, variance};
  }

  CospanMorphism::MappedType
  operator()(Variance variance, FunctorTypeConstructor const &functor) const {
    return CospanMorphism::MappedType{create_default(functor.type), variance};
  }

  CospanMorphism::MappedType
  operator()(Variance variance, TypeConstructor const &type_constructor) const {
    return CospanMorphism::MappedType{create_default(type_constructor.type),
                                      variance};
  }

  template <typename T>
  CospanMorphism::MappedType operator()(Variance variance, T const &) const {
    return CospanMorphism::MappedType{EmptyType(), variance};
  }

} _create_default_mapped_type;

CospanMorphism::MappedType
create_default_type(TypeConstructor::AtomicType const &type) {
  auto const create_cospan_type = std::bind(
      _create_default_mapped_type, type.variance, std::placeholders::_1);
  return std::visit(create_cospan_type, type.type);
}

CospanMorphism
create_default(TypeConstructor::ConstructorType const &constructor) {
  CospanMorphism morphism;
  morphism.map.reserve(constructor.size());
  std::transform(constructor.begin(), constructor.end(),
                 std::back_inserter(morphism.map), create_default_type);
  return std::move(morphism);
}

CospanMorphism create_default_from(TypeConstructor const &constructor) {
  return create_default(constructor.type);
}

} // namespace

namespace Project {
namespace Naturality {

CospanStructure create_default_cospan(Types::TypeConstructor const &domain,
                                      Types::TypeConstructor const &codomain) {
  std::vector<CospanMorphism> domains = {create_default_from(domain),
                                         create_default_from(codomain)};
  return CospanStructure{std::move(domains), {{0, 1}, {1, 0}}, 0, 1};
}

} // namespace Naturality
} // namespace Project