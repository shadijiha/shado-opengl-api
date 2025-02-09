#include "Debug.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <deque>
#include <filesystem>

// ******************** Debug Class ********************
namespace Shado {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::formatter> Log::s_ConsoleFormatter;
	std::shared_ptr<no_duplicate_log_sink> Log::s_Console_sink;

	/**
	 * This class ensures that the log sink is not clustered with repeated messages
	 * If a new log comes that matches the previous log, then we simply update the previous log
	 * without adding the new one to the queue (thus displaying it to the ConsolePanel)
	 *
	 * This class also defines its own spdlog::details::log_msg struct that keeps track of the duplicate count
	 */
	class no_duplicate_log_sink : public spdlog::sinks::base_sink<std::mutex>
	{
		struct log_msg_buffer_with_count : public spdlog::details::log_msg_buffer {
			size_t count;

			log_msg_buffer_with_count() : count(0) {}
			log_msg_buffer_with_count(const spdlog::details::log_msg_buffer& spdlog_msg_buffer, size_t count = 0)
				: spdlog::details::log_msg_buffer(spdlog_msg_buffer), count(count) {}
			log_msg_buffer_with_count(const spdlog::details::log_msg& spdlog_msg_buffer, size_t count = 0)
				: spdlog::details::log_msg_buffer(spdlog_msg_buffer), count(count) {}
		};
		
		std::deque<log_msg_buffer_with_count> sink_;
		size_t max_size_;
		log_msg_buffer_with_count previous_log_;
		
	public:
		no_duplicate_log_sink(size_t n_items)
			: max_size_(n_items)
		{}

		no_duplicate_log_sink(const no_duplicate_log_sink&) = delete;
		no_duplicate_log_sink& operator=(const no_duplicate_log_sink&) = delete;
		
		auto last_formatted() const {
			std::vector<std::string> formatted_messages;
			formatted_messages.reserve(sink_.size());
			for (const auto& log : sink_)	{
				spdlog::memory_buf_t buffer;
				formatter_->format(log, buffer);
				if (log.count > 0)	{
					formatted_messages.push_back("(" + std::to_string(log.count)  + ")  " + std::string(buffer.data(), buffer.size()));
				} else {
					formatted_messages.emplace_back(buffer.data(), buffer.size());
				}
			}
			return formatted_messages;			
		}
		auto clear(){
			sink_.clear();
		}

	protected:
		void sink_it_(const spdlog::details::log_msg &msg) override {
			if (!should_log(msg.level))
				return;
			
			if (msg.payload == previous_log_.payload &&
				msg.level == previous_log_.level &&
				msg.thread_id == previous_log_.thread_id &&
				msg.logger_name == previous_log_.logger_name &&
				msg.source.filename == previous_log_.source.filename &&
				msg.source.line == previous_log_.source.line &&
				!sink_.empty())
			{
				const auto& sink_back_new_count = sink_.back().count + 1;
				sink_.back() = log_msg_buffer_with_count(msg, sink_back_new_count);	// Update it (to update timestamp)
				previous_log_ = log_msg_buffer_with_count(msg, sink_back_new_count);
			} else {
				if (sink_.size() >= max_size_)
					sink_.pop_front();
				sink_.emplace_back(msg);
				previous_log_ = log_msg_buffer_with_count(msg);
			}
		}

		void flush_() override	{
		}
	};
	
	void Log::init()
	{
		// For the editor console
		s_Console_sink = std::make_shared<no_duplicate_log_sink>(100);
		std::filesystem::path fileSinkPath = std::filesystem::absolute("Shado.log");

		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileSinkPath.string(), true));
		logSinks.emplace_back(s_Console_sink);

		s_ConsoleFormatter = std::make_shared<spdlog::pattern_formatter>("%^[%T.%e] %n: %v%$");

		logSinks[0]->set_formatter(s_ConsoleFormatter->clone());
		logSinks[1]->set_pattern("[%Y-%m-%d %T] [%l] %n: %v");
		logSinks[2]->set_formatter(s_ConsoleFormatter->clone());

		s_CoreLogger = std::make_shared<spdlog::logger>("SHADO", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_CoreLogger);
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::err);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_ClientLogger);
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::err);

		SHADO_CORE_INFO("Logging initialized. Log file: {}", fileSinkPath.string());
	}

	std::shared_ptr<spdlog::logger>& Log::getCoreLogger()	{
		return s_CoreLogger;
	}

	std::shared_ptr<spdlog::logger>& Log::getClientLogger()	{
		return s_ClientLogger;
	}

	std::vector<std::string> Log::getMessages()	{
		return s_Console_sink->last_formatted();
	}

	void Log::clearMessages()	{
		s_Console_sink->clear();
	}

	std::shared_ptr<spdlog::formatter> Log::getConsoleFormatter()	{
		return s_ConsoleFormatter;
	}
}
