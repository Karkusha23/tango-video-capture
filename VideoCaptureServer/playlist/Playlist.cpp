#include "Playlist.hpp"

void Playlist::parse_(oatpp::parser::Caret& caret)
{
	records_.clear();
	total_duration_ = 0.0;

	while (caret.canContinue())
	{
		caret.findChar('#');
		if (caret.isAtText("EXTINF:", true))
		{
			v_float64 secs = caret.parseFloat64();
			caret.findROrN();
			caret.skipAllRsAndNs();
			auto uriLabel = caret.putLabel();
			caret.findChar('\n');
			records_.push_back({ secs, uriLabel.toString() });
			total_duration_ += secs;
		}
		caret.findROrN();
		caret.skipAllRsAndNs();
	}
}

Playlist::Playlist(oatpp::parser::Caret& caret)
{
	parse_(caret);
}

Playlist::Playlist(const char* filename)
{
	auto file = oatpp::String::loadFromFile(filename);
	if (file)
	{
		oatpp::parser::Caret caret(file);
		parse_(caret);
	}
	else
	{
		std::cout << "Can not open file!" << std::endl;
	}
}

std::shared_ptr<oatpp::data::stream::BufferOutputStream> Playlist::generateForTime(v_int64 ms, v_int32 records_num)
{
	if (!records_.size())
	{
		throw std::exception("Empty playlist");
	}

	auto stream = std::make_shared<oatpp::data::stream::BufferOutputStream>();

	v_float64 secs = v_float64(ms) / 1000.0;

	v_int64 rounds = std::floor(secs / total_duration_);
	v_float64 loop_time = secs - rounds * total_duration_;
	
	v_float64 cur_time = 0.0;

	v_int64 media_sequence = 1 + rounds * records_.size();

	auto curRecordNode = records_.begin();
	auto lastRecordNode = std::prev(records_.end());

	while (curRecordNode != lastRecordNode)
	{
		if (curRecordNode->duration + cur_time >= loop_time)
		{
			break;
		}

		cur_time += curRecordNode->duration;

		++curRecordNode;
		++media_sequence;
	}

	v_float64 target_duration = 0.0;
	std::list<RecordMarker> list;
	for (v_int32 i = 0; i < records_num; ++i)
	{
		auto& marker = *curRecordNode;
		list.push_back(marker);
		if (marker.duration > target_duration)
		{
			target_duration = marker.duration;
		}
		++curRecordNode;

		if (curRecordNode == records_.end())
		{
			curRecordNode = records_.begin();
		}
	}

	OATPP_LOGD("playlist", "generating sequence {}", media_sequence);

	*stream << "#EXTM3U\n" << "#EXT-X-TARGETDURATION:" << (v_int32)(std::ceil(target_duration)) << "\n" << "#EXT-X-VERSION:3\n" << "#EXT-X-MEDIA-SEQUENCE:" << media_sequence << "\n";

	for (auto& marker : list) 
	{
		*stream << "#EXTINF:" << marker.duration << ",\n" << marker.uri << "\n";
	}

	return stream;
}