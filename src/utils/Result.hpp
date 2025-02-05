#pragma once

#include <cassert>
#include <optional>
#include <variant>

/// \file Result.hpp
/// \brief The file that defines the \ref Result class and the \ref TRY and \ref MUST macros.

// NOTE: Marking these classes as [[nodiscard]] so that whenever we use them we
//       are warned if we don't call them with TRY or MUST.

/// \brief A class that represents a result that can either be a value or an error.
///
/// This class is used in error handling scenarios instead of exceptions.
/// Whenever a function may return an error it will return a \ref Result object that
/// can be checked for the error and handled accordingly.
///
/// The advantage of this approach is that it makes errors more explicit than
/// exceptions by specifying them in the return value and forcing them to handle
/// them.
///
/// Using this class is also made easier by the \ref TRY and \ref MUST macros.
///
/// \tparam ValueType The type of the value that the \ref Result object may contain.
/// \tparam ErrorType The type of the error that the \ref Result object may contain.
template<typename ValueType, typename ErrorType>
class [[nodiscard]] Result {
public:
	/// Construct a \ref Result object with a value.
	/// \param value The value that will be stored in the \ref Result object.
	Result(ValueType&& value)
	  : m_impl(std::forward<ValueType>(value)) {}

	/// Construct a \ref Result object with an error.
	/// \param error The error that will be stored in the \ref Result object.
	Result(ErrorType&& error)
	  : m_impl(std::forward<ErrorType>(error)) {}

	/// Copies a \ref Result object.
	Result(Result const&) = default;
	/// Moves a \ref Result object.
	Result(Result&&) = default;
	/// Destroys a \ref Result object.
	~Result() = default;

	/// Checks if the \ref Result object contains a value.
	///
	/// \return `true` if the \ref Result object contains a value, `false` otherwise.
	bool is_value() const { return std::holds_alternative<ValueType>(m_impl); }

	/// Checks if the \ref Result object contains an error.
	///
	/// \return `true` if the \ref Result object contains a value, `false` otherwise.
	bool is_error() const { return std::holds_alternative<ErrorType>(m_impl); }

	/// Returns the value stored in the \ref Result object.
	/// \note This function will fail if the \ref Result object doesn't contain a value.
	///
	/// \return The value stored in the \ref Result object
	ValueType& value() {
		assert(is_value());
		return std::get<ValueType>(m_impl);
	}

	/// Returns the value stored in the \ref Result object and moves it.
	/// \note This function will fail if the \ref Result object doesn't contain a value.
	///
	/// \return The value stored in the \ref Result object
	ValueType&& release_value() {
		assert(is_value());
		return std::get<ValueType>(std::move(m_impl));
	}

	/// Returns the error stored in the \ref Result object.
	/// \note This function will fail if the \ref Result object doesn't contain an error.
	///
	/// \return The error stored in the \ref Result object
	ErrorType& error() {
		assert(is_error());
		return std::get<ErrorType>(m_impl);
	}

	/// Returns the error stored in the \ref Result object and moves it.
	/// \note This function will fail if the \ref Result object doesn't contain an error.
	///
	/// \return The error stored in the \ref Result object
	ErrorType&& release_error() {
		assert(is_error());
		return std::get<ErrorType>(std::move(m_impl));
	}

private:
	std::variant<ValueType, ErrorType> m_impl;
};

/// \brief A specialization of the \ref Result class for the case where the value is `void`.
///
/// This class is basically an alias for `std::optional<ErrorType>` with the exception
/// it can be used with the \ref TRY and \ref MUST macros.
///
/// \tparam ErrorType The type of the error that the \ref Result object may contain.
template<typename ErrorType>
class Result<void, ErrorType> {
public:
	/// Construct a \ref Result object without an error.
	Result() = default;

	/// Construct a \ref Result object with an error.
	/// \param error The error that will be stored in the \ref Result object.
	Result(ErrorType&& error)
	  : m_impl(std::forward<ErrorType>(error)) {}

	/// Copies a \ref Result object.
	Result(Result const&) = default;
	/// Moves a \ref Result object.
	Result(Result&&) = default;
	/// Destroys a \ref Result object.
	~Result() = default;

	/// Checks if the \ref Result is not an error.
	///
	/// \return `true` if the \ref Result doesn't contain an error, `false` otherwise.
	bool is_value() const { return !is_error(); }
	/// Checks if the \ref Result is an error.
	///
	/// \return `true` if the \ref Result contains an error, `false` otherwise.
	bool is_error() const { return m_impl.has_value(); }

	/// Asserts that the \ref Result is not an error.
	/// \note This method is defined only to reflect the API of the \ref Result class.
	void value() { assert(is_value()); }
	/// Asserts that the \ref Result is not an error.
	/// \note This method is defined only to reflect the API of the \ref Result class.
	void release_value() { assert(is_value()); }

	/// Returns the error stored in the \ref Result object.
	/// \note This function will fail if the \ref Result object doesn't contain an error.
	///
	/// \return The error stored in the \ref Result object
	ErrorType& error() {
		assert(is_error());
		return *m_impl;
	}

	/// Returns the error stored in the \ref Result object and moves it.
	/// \note This function will fail if the \ref Result object doesn't contain an error.
	///
	/// \return The error stored in the \ref Result object
	ErrorType&& release_error() {
		assert(is_error());
		return std::move(*m_impl);
	}

private:
	std::optional<ErrorType> m_impl;
};

/// \def TRY(expr)
/// A macro that is used to check the result of an expression that returns a
/// \ref Result object. If the result is an error, it will return the error,
/// otherwise it will return the value.
///
/// \tparam expr The expression that returns a \ref Result object.
#define TRY(expr)                  \
	({                               \
		auto _tmp = (expr);            \
		if (_tmp.is_error())           \
			return _tmp.release_error(); \
		_tmp.release_value();          \
	})

/// \def MUST(expr)
/// A macro that is used to check the result of an expression that returns a
/// \ref Result object. If the result is an error, it will assert, otherwise it
/// will return the value.
///
/// \tparam expr The expression that returns a \ref Result object.
#define MUST(expr)           \
	({                         \
		auto _tmp = (expr);      \
		assert(_tmp.is_value()); \
		_tmp.release_value();    \
	})
