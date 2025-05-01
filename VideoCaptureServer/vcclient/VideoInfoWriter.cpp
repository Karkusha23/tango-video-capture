#include "VideoInfoWriter.h"
#include <iostream>

namespace vc
{
	VideoInfoWriter::VideoInfoWriter(const std::string& info_path, const std::string& info_base_url) :
		MyThread(20000), info_path_(info_path), header_path_(info_path + "\\ciheader.header"), info_base_url_(info_base_url), file_count_(0)
	{
		infolist1_ = dto::ContourList::createShared();
		infolist1_->list = {};

		std::cout << info_base_url_ << std::endl;

		if (!std::experimental::filesystem::exists(info_path_))
		{
			std::experimental::filesystem::create_directories(info_path_);
		}

		header_ofstream_.open(header_path_);
		header_ofstream_.close();

		start();
	}

	VideoInfoWriter::~VideoInfoWriter()
	{
		stop();
		write_to_file_();
	}

	void VideoInfoWriter::writeContourInfo(const ContourStamp& info)
	{
		std::lock_guard<std::mutex> lock(list_lock_);
		infolist1_->list->push_back(dto::todto(info));
	}

	void VideoInfoWriter::add_to_header_(const std::string& fragmentFilename, int64_t startPts)
	{
		header_ofstream_.open(header_path_, std::ios::app);
		header_ofstream_ << (info_base_url_ + fragmentFilename + " ") << startPts << '\n';
		header_ofstream_.close();
	}

	void VideoInfoWriter::write_to_file_()
	{
		infolist2_ = dto::ContourList::createShared();
		infolist2_->list = {};

		{
			std::lock_guard<std::mutex> lock(list_lock_);
			std::swap(infolist1_, infolist2_);
		}

		if (!infolist2_->list->size())
		{
			return;
		}

		{
			std::lock_guard<std::mutex> lock(file_lock_);
			std::string filename = "cifragment" + std::to_string(++file_count_) + ".json";
			std::ofstream outFile(info_path_ + "\\" + filename);
			outFile << apiObjectMapper_->writeToString(infolist2_->list)->c_str();
			outFile.close();
			add_to_header_(filename, infolist2_->list[0]->pts.getValue(0));
		}
	}

	void VideoInfoWriter::update()
	{
		write_to_file_();
	}
}

namespace dto
{
	oatpp::Object<dto::ContourInfo> todto(const vc::ContourInfo& val)
	{
		oatpp::Object<dto::ContourInfo> result = dto::ContourInfo::createShared();
		result->boundingRect = todto(val.bound_rect);
		result->centerMass = todto(val.center_mass);
		result->areaRel = val.area_rel;
		result->areaAbs = val.area_abs;
		result->diameterRel = val.diameter_rel;
		result->diameterAbs = val.diameter_abs;
		return result;
	}

	oatpp::Object<dto::ContourStamp> todto(const vc::ContourStamp& val)
	{
		oatpp::Object<dto::ContourStamp> result = dto::ContourStamp::createShared();
		result->pts = val.pts;
		result->infos = {};
		for (const vc::ContourInfo& it : val.infos)
		{
			result->infos->push_back(todto(it));
		}
		return result;
	}

	oatpp::Object<dto::ContourList> todto(const std::vector<vc::ContourStamp>& val)
	{
		oatpp::Object<dto::ContourList> result = dto::ContourList::createShared();
		result->list = {};
		for (const vc::ContourStamp& it : val)
		{
			result->list->push_back(todto(it));
		}
		return result;
	}

	vc::ContourInfo fromdto(const oatpp::Object<dto::ContourInfo>& obj)
	{
		return { fromdto(obj->boundingRect), fromdto(obj->centerMass), obj->areaRel.getValue(0), obj->areaAbs.getValue(0), obj->diameterRel.getValue(0), obj->diameterAbs.getValue(0) };
	}

	vc::ContourStamp fromdto(const oatpp::Object<dto::ContourStamp>& obj)
	{
		std::vector<vc::ContourInfo> infos;
		for (const oatpp::Object<dto::ContourInfo>& it : *(obj->infos))
		{
			infos.push_back(fromdto(it));
		}
		return { obj->pts.getValue(0), infos };
	}

	std::vector<vc::ContourStamp> fromdto(const oatpp::Object<dto::ContourList>& obj)
	{
		std::vector<vc::ContourStamp> result;
		for (const oatpp::Object<dto::ContourStamp>& it : *(obj->list))
		{
			result.push_back(fromdto(it));
		}
		return result;
	}
}