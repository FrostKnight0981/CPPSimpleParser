#include "node.h"

#include <iostream>

#pragma region NodeValue

NodeValue::NodeValue() : m_Data(nullptr), m_Type(NodeValue::Type::None)
{ }

NodeValue::NodeValue(const NodeValue& other) : NodeValue()
{
	Copy(other);
}

NodeValue::NodeValue(NodeValue&& other) noexcept : NodeValue()
{
	Move(std::move(other));
}

NodeValue& NodeValue::operator=(const NodeValue& other)
{
	if (this == &other)
		return *this;

	Copy(other);
	return *this;
}

NodeValue& NodeValue::operator=(NodeValue&& other) noexcept
{
	if (this == &other)
		return *this;

	Move(std::move(other));
	return *this;
}

NodeValue::~NodeValue()
{
	Clear();
}

void NodeValue::Move(NodeValue&& other)
{
	Clear();

	m_Data = other.m_Data;
	m_Type = other.m_Type;

	other.m_Type = Type::None;
	other.m_Data = nullptr;
}

void NodeValue::Copy(const NodeValue& other)
{
	Clear();

	// Can be simplified with magic_enum library
	switch (other.m_Type)
	{
	case Type::List:
	{
		using NativeType = internal::node_to_native<Type::List>::Type;
		Set(*reinterpret_cast<NativeType*>(other.m_Data));
		break;
	}
	case Type::String:
	{
		using NativeType = internal::node_to_native<Type::String>::Type;
		Set(*reinterpret_cast<NativeType*>(other.m_Data));
		break;
	}
	}
}

void NodeValue::Clear()
{
	// Memory freeing for pointer types
	// Can be simplified with magic_enum library
	switch (m_Type)
	{
	case Type::List:
	{
		auto pValue = reinterpret_cast<internal::node_to_native<Type::List>::Type*>(m_Data);
		delete pValue;
		break;
	}
	case Type::String:
	{
		auto pValue = reinterpret_cast<internal::node_to_native<Type::String>::Type*>(m_Data);
		delete pValue;
		break;
	}
	}

	m_Data = nullptr;
	m_Type = Type::None;
}

std::string NodeValue::AsString() const
{
	return Get<Type::String>();
}

std::vector<Node> NodeValue::AsList() const
{
	return Get<Type::List>();
}

NodeValue::Type NodeValue::GetType() const
{
	return m_Type;
}

#pragma endregion NodeValue

#pragma region Node

Node::Node(uint32_t id, const std::string& name) :
	m_Id(id), m_Name(name), m_Value()
{ }

Node::Node(uint32_t id, const std::string& name, NodeValue&& value) : 
	m_Id(id), m_Name(name), m_Value(std::move(value))
{ }

Node::Node(uint32_t id, const std::string& name, const NodeValue& value) :
	m_Id(id), m_Name(name), m_Value(value)
{ }

uint32_t Node::GetId() const
{
	return m_Id;
}

std::string Node::GetName() const
{
	return m_Name;
}

void Node::SetValue(NodeValue&& value)
{
	m_Value = std::move(value);
}

void Node::SetValue(const NodeValue& value)
{
	m_Value = value;
}

NodeValue Node::GetValue() const
{
	return m_Value;
}

Node& Node::operator=(NodeValue&& value)
{
	m_Value = std::move(value);
}

Node& Node::operator=(const NodeValue& value)
{
	m_Value = value;
}

#pragma endregion Node

#pragma region Utils

void node_util::PrintNode(std::ostream* pStream, const Node& node, int depth /*= 0*/, uint32_t parentId /*= 0*/)
{
	auto& outStream = *pStream;

	for (int i = 0; i < depth; i++)
	{
		outStream << "  ";
	}

	outStream << "(" << node.GetId() << ", " << parentId << ", " << node.GetName() << ", ";
	
	auto value = node.GetValue();
	switch (value.GetType())
	{
	case NodeValue::Type::String:
		outStream << value.AsString();
		break;
	case NodeValue::Type::List:
	{
		auto list = value.AsList();
		outStream << "[";
		for (auto it = list.begin(); it != list.end(); it++)
		{
			if (it != list.begin())
				outStream << ", ";
			outStream << it->GetId();
		}
		outStream << "]";
		break;
	}
	default:
		outStream << "Undefined";
		break;
	}

	outStream << ")" << std::endl;

	if (value.GetType() == NodeValue::Type::List)
	{
		auto list = value.AsList();
		for (auto& entry : list)
		{
			PrintNode(pStream, entry, depth + 1, node.GetId());
		}
	}
}

#pragma endregion Utils
