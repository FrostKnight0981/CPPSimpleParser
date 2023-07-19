#pragma once
#include <vector>
#include <string>
#include <stdexcept>

class Node;

class NodeValue
{
public:
	enum class Type
	{
		None,
		String,
		List
	};

public:
	template<class TValue>
	NodeValue(TValue&& value);

	template<class TValue>
	NodeValue& operator=(TValue&& value);

	template<class TValue>
	void Set(TValue&& value);

	std::string AsString() const;
	std::vector<Node> AsList() const;

	Type GetType() const;

private:

	template<NodeValue::Type eType>
	inline auto Get() const;

private:
	void Move(NodeValue&& other);
	void Copy(const NodeValue& other);

	void Clear();

private:
	Type m_Type;
	void* m_Data;

public:
	NodeValue();

	NodeValue(const NodeValue& value);
	NodeValue(NodeValue&& value) noexcept;

	NodeValue& operator=(const NodeValue& other);
	NodeValue& operator=(NodeValue&& other) noexcept;

	~NodeValue();
};

class Node
{
private:
	uint32_t m_Id;
	std::string m_Name;
	NodeValue m_Value;

public:
	Node(uint32_t id, const std::string& name);

	Node(uint32_t id, const std::string& name, NodeValue&& value);
	Node(uint32_t id, const std::string& name, const NodeValue& value);

	inline Node& operator=(const NodeValue& value);
	inline Node& operator=(NodeValue&& value);

	inline uint32_t GetId() const;
	inline std::string GetName() const;

	inline void SetValue(NodeValue&& value);
	inline void SetValue(const NodeValue& value);

	inline NodeValue GetValue() const;
};

namespace node_util
{
	// Can be created the interface with Printer functionality to give able custom the printer.
	// But for this task I think enough what we have.
	void PrintNode(std::ostream* pStream, const Node& node, int depth = 0, uint32_t parentId = 0);
}

namespace internal
{
	template<class TValue> struct native_to_node {};
	template<> struct native_to_node<std::string> : std::integral_constant<NodeValue::Type, NodeValue::Type::String> {};
	template<> struct native_to_node<std::vector<Node>> : std::integral_constant<NodeValue::Type, NodeValue::Type::List> {};

	template<NodeValue::Type eType> struct node_to_native {};
	template<> struct node_to_native<NodeValue::Type::String> { using Type = std::string; };
	template<> struct node_to_native<NodeValue::Type::List> { using Type = std::vector<Node>; };
}

// Template implementation

template<class TValue>
NodeValue::NodeValue(TValue&& value) : NodeValue()
{
	Set(std::forward<TValue>(value));
}

template<class TValue>
NodeValue& NodeValue::operator=(TValue&& value)
{
	Set(std::forward<TValue>(value));
}

template<class TValue>
void NodeValue::Set(TValue&& value)
{
	// In our current task we don't need the types below 8 bytes size
    // But if it will be needed, to effective use memory, it would be saved in m_Data, not by creating the pointer

	using ClearType = std::decay_t<TValue>;
	constexpr Type nodeType = internal::native_to_node<ClearType>::value;

	Clear();

	auto pValue = new ClearType();
	*pValue = std::forward<TValue>(value);
	m_Data = pValue;
	m_Type = nodeType;
}

template<NodeValue::Type eType>
auto NodeValue::Get() const
{
	if (m_Type != eType)
	{
		throw std::logic_error("Trying to get value from container with wrong type.");
	}

	using NativeType = typename internal::node_to_native<eType>::Type;
	return *reinterpret_cast<NativeType*>(m_Data);
}
