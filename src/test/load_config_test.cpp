#include "gtest/gtest.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "log4cpp.hpp"
#include "main/log4cpp_config.h"

class TestEnvironment: public testing::Environment {
public:
	explicit TestEnvironment(const std::string &cur_path) {
		size_t end = cur_path.find_last_of('\\');
		if (end == std::string::npos) {
			end = cur_path.find_last_of('/');
		}
		const std::string work_dir = cur_path.substr(0, end);
		std::filesystem::current_path(work_dir);
	}
};

int main(int argc, char **argv) {
	const std::string cur_path = argv[0];
	testing::InitGoogleTest(&argc, argv);
	AddGlobalTestEnvironment(new TestEnvironment(cur_path));
	return RUN_ALL_TESTS();
}

void layout_pattern_cfg_check(const nlohmann::json &expected_json, const std::string &layout_pattern) {
	const std::string expected = expected_json.at("layout_pattern");
	EXPECT_EQ(layout_pattern, expected);
}

void console_appender_cfg_check(const nlohmann::json &console_appender, const log4cpp::console_appender_config *cfg) {
	const std::string actual = cfg->get_out_stream();
	const std::string expected = console_appender.at("out_stream");
	EXPECT_EQ(actual, expected);
}

void file_appender_cfg_check(const nlohmann::json &file_appender, const log4cpp::file_appender_config *cfg) {
	const std::string actual = cfg->get_file_path();
	const std::string expected = file_appender.at("file_path");
	EXPECT_EQ(actual, expected);
}

void tcp_appender_cfg_check(const nlohmann::json &tcp_appender, const log4cpp::tcp_appender_config *cfg) {
	const log4cpp::net::net_addr actual_addr = cfg->get_local_addr();
	const log4cpp::net::net_addr expected_addr = log4cpp::net::net_addr(tcp_appender.at("local_addr"));
	EXPECT_EQ(actual_addr, expected_addr);
	unsigned short actual_port = cfg->get_port();
	unsigned short expected_port = tcp_appender.at("port");
	EXPECT_EQ(actual_port, expected_port);
}

void udp_appender_cfg_check(const nlohmann::json &udp_appender, const log4cpp::udp_appender_config *cfg) {
	const log4cpp::net::net_addr actual_addr = cfg->get_local_addr();
	const log4cpp::net::net_addr expected_addr = log4cpp::net::net_addr(udp_appender.at("local_addr"));
	EXPECT_EQ(actual_addr, expected_addr);
	unsigned short actual_port = cfg->get_port();
	unsigned short expected_port = udp_appender.at("port");
	EXPECT_EQ(actual_port, expected_port);
}

void appenders_cfg_check(const nlohmann::json &appenders_json, const log4cpp::appender_config &appenders_cfg) {
	const nlohmann::json &appenders = appenders_json.at("appenders");
	// Console Appender
	const log4cpp::console_appender_config *console_appender_cfg = appenders_cfg.get_console_cfg();
	ASSERT_EQ(true == appenders.contains("console_appender"), nullptr != console_appender_cfg);
	if (nullptr != console_appender_cfg) {
		const nlohmann::json &console_appender = appenders.at("console_appender");
		console_appender_cfg_check(console_appender, console_appender_cfg);
	}
	// File Appender
	const log4cpp::file_appender_config *file_appender_cfg = appenders_cfg.get_file_cfg();
	ASSERT_EQ(true == appenders.contains("file_appender"), nullptr != file_appender_cfg);
	if (nullptr != file_appender_cfg) {
		const nlohmann::json &file_appender = appenders.at("file_appender");
		file_appender_cfg_check(file_appender, file_appender_cfg);
	}
	// TCP Appender
	const log4cpp::tcp_appender_config *tcp_appender_cfg = appenders_cfg.get_tcp_cfg();
	ASSERT_EQ(true == appenders.contains("tcp_appender"), nullptr != tcp_appender_cfg);
	if (nullptr != tcp_appender_cfg) {
		const nlohmann::json &tcp_appender = appenders.at("tcp_appender");
		tcp_appender_cfg_check(tcp_appender, tcp_appender_cfg);
	}
	// UDP Appender
	const log4cpp::udp_appender_config *udp_appender_cfg = appenders_cfg.get_udp_cfg();
	ASSERT_EQ(true == appenders.contains("udp_appender"), nullptr != udp_appender_cfg);
	if (nullptr != udp_appender_cfg) {
		const nlohmann::json &udp_appender = appenders.at("udp_appender");
		udp_appender_cfg_check(udp_appender, udp_appender_cfg);
	}
}

unsigned char appender_name_to_flag(const std::vector<std::string> &appenders) {
	unsigned char appenders_flag = 0;
	for (const std::string &expected_appender: appenders) {
		if (log4cpp::CONSOLE_APPENDER_NAME == expected_appender) {
			appenders_flag |= log4cpp::CONSOLE_APPENDER_FLAG;
		}
		if (log4cpp::FILE_APPENDER_NAME == expected_appender) {
			appenders_flag |= log4cpp::FILE_APPENDER_FLAG;
		}
		if (log4cpp::TCP_APPENDER_NAME == expected_appender) {
			appenders_flag |= log4cpp::TCP_APPENDER_FLAG;
		}
		if (log4cpp::UDP_APPENDER_NAME == expected_appender) {
			appenders_flag |= log4cpp::UDP_APPENDER_FLAG;
		}
	}
	return appenders_flag;
}

void appender_flag_to_name(unsigned char appenders_flag, std::vector<std::string> &appenders) {
	if (0 != (appenders_flag & log4cpp::CONSOLE_APPENDER_FLAG)) {
		appenders.emplace_back(log4cpp::CONSOLE_APPENDER_NAME);
	}
	if (0 != (appenders_flag & log4cpp::FILE_APPENDER_FLAG)) {
		appenders.emplace_back(log4cpp::FILE_APPENDER_NAME);
	}
	if (0 != (appenders_flag & log4cpp::TCP_APPENDER_FLAG)) {
		appenders.emplace_back(log4cpp::TCP_APPENDER_NAME);
	}
	if (0 != (appenders_flag & log4cpp::UDP_APPENDER_FLAG)) {
		appenders.emplace_back(log4cpp::UDP_APPENDER_NAME);
	}
}

namespace log4cpp {
	void from_json(const nlohmann::json &json, layout_config &obj) {
		obj.set_name(json.at("name"));
		obj.set_level(from_string(json.at("log_level")));
		std::vector<std::string> appenders = json.at("appenders");
		unsigned char appenders_flag = appender_name_to_flag(appenders);
		obj.set_layout_flag(appenders_flag);
	}

	void to_json(nlohmann::json &json, const layout_config &obj) {
		std::vector<std::string> appenders;
		appender_flag_to_name(obj.get_layout_flag(), appenders);
		json = nlohmann::json{
			{"name", obj.get_logger_name()}, {"log_level", obj.get_logger_level()}, {"appenders", appenders}};
	}
}

void layout_cfg_check(const nlohmann::json &expected_json, const std::vector<log4cpp::layout_config> &actual_layouts) {
	EXPECT_EQ(actual_layouts.empty(), true != expected_json.contains("layouts"));
	if (actual_layouts.empty()) {
		return;
	}
	const nlohmann::json &layouts_json = expected_json.at("layouts");
	const std::vector<log4cpp::layout_config> expected_layouts =
		layouts_json.get<std::vector<log4cpp::layout_config>>();
	EXPECT_EQ(actual_layouts, expected_layouts);
}

void root_layout_cfg_check(const nlohmann::json &expected_json, const log4cpp::layout_config &root_layout_cfg) {
	const nlohmann::json &root_layout = expected_json.at("root_layout");
	const std::string actual_name = root_layout_cfg.get_logger_name();
	EXPECT_EQ(actual_name, "root");
	log4cpp::log_level actual_level = root_layout_cfg.get_logger_level();
	log4cpp::log_level expected_level = log4cpp::from_string(root_layout.at("log_level"));
	EXPECT_EQ(actual_level, expected_level);
	unsigned char actual_appenders_flag = root_layout_cfg.get_layout_flag();
	const std::vector<std::string> expected_appenders = root_layout.at("appenders");
	unsigned char expected_appenders_flag = appender_name_to_flag(expected_appenders);
	EXPECT_EQ(actual_appenders_flag, expected_appenders_flag);
}

void parse_json(const std::string &config_file, nlohmann::json &expected_json) {
	std::ifstream ifs(config_file);
	ASSERT_EQ(ifs.is_open(), true);
	expected_json = nlohmann::json::parse(ifs);
	ifs.close();
}

void configuration_cfg_check(const nlohmann::json &expected_json, const log4cpp::log4cpp_config *config) {
	// Layout pattern
	const std::string &layout_pattern = config->get_layout_pattern();
	layout_pattern_cfg_check(expected_json, layout_pattern);
	// Appenders
	const log4cpp::appender_config &appender_cfg = config->get_appender();
	appenders_cfg_check(expected_json, appender_cfg);
	// Layouts
	std::vector<log4cpp::layout_config> layouts_cfg = config->get_layouts();
	std::sort(layouts_cfg.begin(), layouts_cfg.end());
	layout_cfg_check(expected_json, layouts_cfg);
	// Root layout
	const log4cpp::layout_config &root_layout_cfg = config->get_root_layout();
	root_layout_cfg_check(expected_json, root_layout_cfg);
}

TEST(load_config_test, auto_load_config) {
	// Just to load the configuration file
	std::shared_ptr<log4cpp::layout> layout = log4cpp::layout_manager::get_layout("console_layout");
	const log4cpp::log4cpp_config *config = log4cpp::layout_manager::get_config();
	ASSERT_NE(nullptr, config);

	nlohmann::json expected_json;
	parse_json("log4cpp.json", expected_json);
	configuration_cfg_check(expected_json, config);
}

TEST(load_config_test, load_config_test_1) {
	const std::string config_file = "log4cpp_config_1.json";
	log4cpp::layout_manager::load_config(config_file);
	const log4cpp::log4cpp_config *config = log4cpp::layout_manager::get_config();
	ASSERT_NE(nullptr, config);

	nlohmann::json expected_json;
	parse_json(config_file, expected_json);
	configuration_cfg_check(expected_json, config);
}

TEST(load_config_test, load_config_test_2) {
	const std::string config_file = "log4cpp_config_2.json";
	log4cpp::layout_manager::load_config(config_file);
	const log4cpp::log4cpp_config *config = log4cpp::layout_manager::get_config();
	ASSERT_NE(nullptr, config);

	nlohmann::json expected_json;
	parse_json(config_file, expected_json);
	configuration_cfg_check(expected_json, config);
}
