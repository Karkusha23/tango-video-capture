#ifndef MyDTOs_hpp_included
#define MyDTOs_hpp_included

#include <oatpp/core/Types.hpp>
#include <oatpp/core/macro/codegen.hpp>

#include <opencv2/core.hpp>
#include <vc/camproc.h>

#include OATPP_CODEGEN_BEGIN(DTO)

namespace dto
{
	class Point : public oatpp::DTO
	{
		DTO_INIT(Point, DTO);

		DTO_FIELD(Int32, x);
		DTO_FIELD(Int32, y);
	};

	class Rect : public oatpp::DTO
	{
		DTO_INIT(Rect, DTO);

		DTO_FIELD(Object<dto::Point>, tl);
		DTO_FIELD(Object<dto::Point>, br);
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

	class ContourInfo : public oatpp::DTO
	{
		DTO_INIT(ContourInfo, DTO);

		DTO_FIELD(Object<dto::Rect>, boundingRect);
		DTO_FIELD(Object<dto::Point>, centerMass);
		DTO_FIELD(Int32, areaRel);
		DTO_FIELD(Float64, areaAbs);
		DTO_FIELD(Float64, diameterRel);
		DTO_FIELD(Float64, diameterAbs);
	};

	class ContourStamp : public oatpp::DTO
	{
		DTO_INIT(ContourStamp, DTO);

		DTO_FIELD(Int64, pts);
		DTO_FIELD(List<Object<dto::ContourInfo>>, infos);
	};

	class ContourList : public oatpp::DTO
	{
		DTO_INIT(ContourList, DTO);

		DTO_FIELD(List<Object<dto::ContourStamp>>, list);
	};
}

#include OATPP_CODEGEN_END(DTO)

namespace dto
{
	oatpp::Object<dto::Point> todto(const cv::Point& val);
	oatpp::Object<dto::Rect> todto(const cv::Rect& val);
	oatpp::Object<dto::Ruler> todto(const vc::Ruler& val);

	cv::Point fromdto(const oatpp::Object<dto::Point>& obj);
	cv::Rect fromdto(const oatpp::Object<dto::Rect>& obj);
	vc::Ruler fromdto(const oatpp::Object<dto::Ruler>& obj);
}

#endif