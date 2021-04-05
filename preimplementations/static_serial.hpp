#include <tuple>
#include <boost/type_index.hpp>

#include <iostream>
#include <fstream>
namespace serial
{
	struct ___null_t
	{
		void serialize(std::ostream &, const void *) const {}
		bool serialize_coma_separated(std::ostream &, const void *) const { return false; }
		void deserialize(std::istream &, const void *) const {}
	};

	struct SerialHelper
	{
		___null_t _{};
	};

	template <auto value, typename T>
	struct ser
	{
	};

	constexpr char delimiter{' '};

	template <typename Class, typename Result, Result Class::*value, typename T>
	struct ser<value, T>
	{

		using value_type = T;
		value_type val;

		template <typename... U>
		explicit ser(U &&...u) : val{std::forward<U>(u)...} {}

		ser(T &&v) : val{v} {};
		ser(const T &v) : val{v} {};

		template <typename U>
		ser(const U &v) : val{v} {};

		void serialize(std::ostream &os, const Class *that) const
		{
			(that->*value).serialize(os, that);
			os << this->val << delimiter;
		}

		bool serialize_coma_separated(std::ostream &os, const Class *that) const
		{
			if ((that->*value).serialize_coma_separated(os, that))
				os << ", ";
			os << this->val;
			return true;
		}

		void deserialize(std::istream &is, Class *that)
		{
			(that->*value).deserialize(is, that);
			// is >> this->val;
			is >> val;
			is.ignore(1, delimiter);
		}
	};

	template <auto LastItemRef>
	struct ClassWrapper
	{
	};

	template <typename Class, typename Result, Result Class::*last>
	struct ClassWrapper<last>
	{
		using class_t = Class;

		class_t val;

		template <typename... U>
		explicit ClassWrapper(U &&...u) : val{___null_t{}, std::forward<U>(u)...} {}

		void serialize(std::ostream &os) const
		{
			(val.*last).serialize(os, &val);
		}

		void deserialize(std::istream &is)
		{
			(val.*last).deserialize(is, &val);
		}

		void serialize_coma_separated(std::ostream &os) const
		{
			(val.*last).serialize_coma_separated(os, &val);
		}
	};

	struct _Example : public SerialHelper
	{
		ser<&_Example::_, int> a{1};
		ser<&_Example::a, int> b{2};
		ser<&_Example::b, int> c{3};
	};
	using Example = ClassWrapper<&_Example::c>;
};

template <auto T>
inline std::ostream &operator<<(std::ostream &os, const typename serial::ClassWrapper<T> &obj)
{
	obj.serialize(os);
	return os;
}

template <auto T>
inline std::istream &operator>>(std::istream &is, typename serial::ClassWrapper<T> &obj)
{
	obj.deserialize(is);
	return is;
}

template <auto T>
struct pretty_print
{
	using wrap_t = typename serial::ClassWrapper<T>;
	const wrap_t &ref;
	explicit pretty_print(const wrap_t &obj) : ref{obj} {}
	inline friend std::ostream &operator<<(std::ostream &os, const pretty_print<T> &obj)
	{
		os << boost::typeindex::type_id<typename wrap_t::class_t>();
		os << "[ ";
		obj.ref.serialize_coma_separated(os);
		os << " ]";
		return os;
	}
};