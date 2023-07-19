#pragma once
#include <string>
#include <optional>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <node.h>

namespace internal
{
	class Token
	{
	public:
		enum class Type
		{
			None,
			Name,
			String,
			ArrayOpen,
			ArrayClose,
			Assign
		};

	public:
		Token();
		Token(Type type);
		Token(Type type, const std::string& value);

		Type GetType() const;
		std::string GetValue() const;

	private:
		Type m_Type;
		std::string m_Value;
	};

	class Tokenizer
	{
	public:
		Tokenizer();
		Tokenizer(std::ifstream&& stream);
		Tokenizer(std::istringstream&& stream);

		Token NextToken();
		Token PeekToken();

	private:
		bool ReadName(std::string& out);
		bool ReadString(std::string& out);
		bool IsEmpty();

		std::unique_ptr<std::istream> m_Stream;
	};
}

class Parser
{
public:
	std::optional<Node> Parse(const std::filesystem::path& filepath);
	std::optional<Node> Parse(const std::string& data);

private:
	uint32_t m_LastId;
	internal::Tokenizer m_Tokenizer;

private:
	std::optional<Node> ParseNode();
	std::optional<NodeValue> ParseValue();
};
