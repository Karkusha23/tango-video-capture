#ifndef StaticFileManager_hpp_included
#define StaticFileManager_hpp_included

#include <oatpp/core/concurrency/SpinLock.hpp>
#include <oatpp/core/Types.hpp>

#include <unordered_map>
#include <string>

oatpp::String getFileExtension(const oatpp::String& filename);

class StaticFileManager
{
public:

	StaticFileManager(const oatpp::String& path);

	static std::shared_ptr<StaticFileManager> createShared(const oatpp::String& path);

	oatpp::String getFile(const oatpp::String& filename, bool ignore_cache = false);
	oatpp::String getMimeType(const oatpp::String& filename);

private:

	oatpp::String path_;
	oatpp::concurrency::SpinLock lock_;
	std::unordered_map<oatpp::String, oatpp::String> cache_;
};

oatpp::String formatText(const char* text, ...);

#endif