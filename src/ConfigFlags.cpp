#include "ConfigFlags.h"
#include "CommandError.h"
#include "Utils/FlagUtils.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace
{
	template <typename T>
	std::optional<T> ParseValue(std::string_view value)
	{
		if constexpr (std::same_as<T, bool>)
		{
			if (value == "true" || value == "1")
			{
				return true;
			}
			else if (value == "false" || value == "0")
			{
				return false;
			}

			return std::nullopt;
		}
		else if constexpr (std::same_as<T, BuildType>)
		{
			BuildType type = Config::StringToBuildType(std::string{value});

			if (type == BuildType::Invalid)
			{
				return std::nullopt;
			}

			return type;
		}
		else if constexpr (std::same_as<T, std::string>)
		{
			return std::string{value};
		}
		else if constexpr (std::is_arithmetic_v<T>)
		{
			T result{};
			const char* end = value.data() + value.size();
			auto [ptr, ec] = std::from_chars(value.data(), end, result);

			if (ec != std::errc{} || ptr != end)
			{
				return std::nullopt;
			}

			return result;
		}
		else
		{
			static_assert(sizeof(T) == 0, "Unsupported flag value type");
		}
	}

	struct Binding
	{
		std::string_view name;
		std::function<bool(Config* config, std::string_view value)> apply;
	};

	template<typename Section, typename T>
	Binding Bind(std::string_view name, Section Config::* section, T Section::* field)
	{
		return Binding{name, [section, field](Config* config, std::string_view value)
		{
			auto parsedValue = ParseValue<T>(value);
			if (! parsedValue)
			{
				return false;
			}

			config->*section.*field = *parsedValue;

			return true;
		}};
	}

	std::string_view StripQuotes(std::string_view value)
	{
		if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
		{
			return value.substr(1, value.size() - 2);
		}

		return value;
	}
}

CommandError ConfigFlags::Apply(Config *config, const std::vector<std::string> &flags)
{
	static const std::array bindings
	{
		Bind("type",               &Config::buildConfig, &BuildConfig::type),
		Bind("cpp-version",        &Config::buildConfig, &BuildConfig::cppVersion),
		Bind("source-directory",   &Config::buildConfig, &BuildConfig::sourceDirectory),
		Bind("header-directory",   &Config::buildConfig, &BuildConfig::headerDirectory),
		Bind("build-system",       &Config::buildConfig, &BuildConfig::buildSystem),
		Bind("build-shared-libs",  &Config::buildConfig, &BuildConfig::buildSharedLibs),
		Bind("enable-warnings",    &Config::buildConfig, &BuildConfig::enableWarnings),
		Bind("warnings-as-errors", &Config::buildConfig, &BuildConfig::warningsAsErrors),
		Bind("enable-sanitizers",  &Config::buildConfig, &BuildConfig::enableSanitizers),
		Bind("enable-lto",         &Config::buildConfig, &BuildConfig::enableLto),
		Bind("enable-ccache",      &Config::buildConfig, &BuildConfig::enableCcache),
		
		Bind("preserve-cache", &Config::extrasConfig, &ExtrasConfig::preserveCache)
	};

	for (const auto& flag : flags)
	{
		if (! Utils::HasFlagPrefix(flag))
		{
			return CommandError{false, std::format("Unexpected argument '{}', command flags must start with --", flag)};
		}

		std::string_view body = std::string_view(flag).substr(2);
		size_t equals = body.find('=');

		std::string_view name  = body.substr(0, equals);
		std::string_view value = equals == std::string_view::npos ? std::string_view("true") : StripQuotes(body.substr(equals + 1));

		auto binding = std::ranges::find(bindings, name, &Binding::name);
		if (binding == bindings.end())
		{
			return CommandError{false, std::format("Unknow flag '--{}'", name)};
		}

		if (! binding->apply(config, value))
		{
			return CommandError{false, std::format("Invalid value '{}' for flag '--{}'", value, name)};
		}
	}

	return CommandError{true, "No errors occured"};
}
