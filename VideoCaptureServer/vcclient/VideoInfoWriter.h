#ifndef VideoInfoWriter_h_included
#define VideoInfoWriter_h_included

#include <fstream>
#include <experimental/filesystem>

#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/core/macro/component.hpp>

#include <vc/camproc.h>

#include "../mythread/MyThread.h"
#include "../dtos/MyDTOs.hpp"

namespace vc
{
	struct ContourStamp
	{
		int64_t pts;
		std::vector<ContourInfo> infos;
	};

	class VideoInfoWriter : public MyThread
	{
	public:

		VideoInfoWriter(const std::string& info_path, const std::string& info_base_url);
		virtual ~VideoInfoWriter();

		void writeContourInfo(const ContourStamp& info);

	private:

		const std::string info_path_;
		const std::string header_path_;
		const std::string info_base_url_;

		OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper_);

		oatpp::Object<dto::ContourList> infolist1_;
		oatpp::Object<dto::ContourList> infolist2_;

		int file_count_;

		std::mutex list_lock_;
		std::mutex file_lock_;

		std::ofstream header_ofstream_;

		void add_to_header_(const std::string& fragmentFilename, int64_t startPts);
		void write_to_file_();

		void update() override;
	};
}

namespace dto
{
	oatpp::Object<dto::ContourInfo> todto(const vc::ContourInfo& val);
	oatpp::Object<dto::ContourStamp> todto(const vc::ContourStamp& val);
	oatpp::Object<dto::ContourList> todto(const std::vector<vc::ContourStamp>& val);

	vc::ContourInfo fromdto(const oatpp::Object<dto::ContourInfo>& obj);
	vc::ContourStamp fromdto(const oatpp::Object<dto::ContourStamp>& obj);
	std::vector<vc::ContourStamp> fromdto(const oatpp::Object<dto::ContourList>& obj);
}

#endif