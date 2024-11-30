#ifndef StaticFileManager_hpp_included
#define StaticFileManager_hpp_included

#include <oatpp/core/concurrency/SpinLock.hpp>
#include <oatpp/core/Types.hpp>

#include <unordered_map>
#include <string>
#include <experimental/filesystem>

oatpp::String getFileExtension(const oatpp::String& filename);
oatpp::String formatText(const char* text, ...);
oatpp::String getMimeType(const oatpp::String& filename);

class StaticFileManager
{
public:

	StaticFileManager(const oatpp::String& path);

	static std::shared_ptr<StaticFileManager> createShared(const oatpp::String& path);

	oatpp::String getFile(const oatpp::String& filename, bool ignore_cache = false);
	bool isFileExists(const std::string& filename);

private:

	oatpp::String path_string_;
	std::experimental::filesystem::path path_;
	oatpp::concurrency::SpinLock lock_;
	std::unordered_map<oatpp::String, oatpp::String> cache_;
};

#endif