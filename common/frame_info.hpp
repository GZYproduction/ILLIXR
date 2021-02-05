#pragma once
#include <string>
#include "../runtime/sqlite_wrapper.hpp"

namespace ILLIXR {

	static const sqlite::schema site_info_schema {{
		{"plugin_id", sqlite::type_INTEGER},
		{"topic_name", sqlite::type_INTEGER},
		{"serial_no", sqlite::type_INTEGER},
	}};

	class FrameInfo {
	private:
		size_t plugin;
		std::string topic;
		size_t serial_no;
	public:
		FrameInfo(size_t plugin_ = 0, std::string topic_ = std::string{}, size_t serial_no_ = 0)
			: plugin{plugin_}
			, topic{std::move(topic_)}
			, serial_no{serial_no_}
		{ }
		void serialize(std::vector<sqlite::value>& row) const {
			row.emplace_back(plugin);
			row.emplace_back(std::string_view{topic});
			row.emplace_back(serial_no);
		}
	};

	static const FrameInfo default_frame_info;
}