#ifndef MyDTOs_hpp_included
#define MyDTOs_hpp_included

#include <oatpp/core/Types.hpp>
#include <oatpp/core/macro/codegen.hpp>

#include OATPP_CODEGEN_BEGIN(DTO)

namespace dto
{
	class Point : public oatpp::DTO
	{
		DTO_INIT(Point, DTO);

		DTO_FIELD(Int32, x);
		DTO_FIELD(Int32, y);
	};

	class Ruler : public oatpp::DTO
	{
		DTO_INIT(Ruler, DTO);

		DTO_FIELD(Object<dto::Point>, start);
		DTO_FIELD(Object<dto::Point>, end);
		DTO_FIELD(Float64, length);
	};

	class VCParams : public oatpp::DTO
	{
		DTO_INIT(VCParams, DTO);

		DTO_FIELD(Object<dto::Ruler>, ruler);
		DTO_FIELD(Int32, threshold);
		DTO_FIELD(Float64, minContourArea);
	};
}

#include OATPP_CODEGEN_END(DTO)

#endif