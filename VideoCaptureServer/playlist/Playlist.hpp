#ifndef Playlist_hpp_included
#define Playlist_hpp_included

#include <oatpp/core/data/stream/BufferStream.hpp>
#include <oatpp/core/parser/Caret.hpp>

#include <chrono>
#include <list>
#include <iostream>

class Playlist
{
	struct RecordMarker
	{
		v_float64 duration;
		oatpp::String uri;
	};

	std::list<RecordMarker> records_;
	v_float64 total_duration_;

	void parse_(oatpp::parser::Caret& caret);

public:

	Playlist(oatpp::parser::Caret& caret);
	Playlist(const char* filename);

	std::shared_ptr<oatpp::data::stream::BufferOutputStream> generateForTime(v_int64 ms, v_int32 records_num);
};

#endif