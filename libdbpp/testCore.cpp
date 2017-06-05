#include "testCore.h"
#include <selectcommand.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/utility/enable_if.hpp>
#include <compileTimeFormatter.h>

namespace DB {

TestCore::TestCore() :
	testInt(43),
	testDouble(3.14),
	testString("Some C String"),
	testBool(false),
	testDateTime(boost::posix_time::from_time_t(1430530593)),
	testInterval(boost::posix_time::time_duration(1, 2, 3))
{
}

template<typename T>
class Assert : public DB::HandleField {
	public:
		Assert(const T & e) : expected(e) { }

		void floatingpoint(double v) override { (*this)(v); }
		void integer(int64_t v) override { (*this)(v); }
		void boolean(bool v) override { (*this)(v); }
		void string(const char * v, size_t len) override { (*this)(std::string(v, len)); }
		void timestamp(const boost::posix_time::ptime & v) override { (*this)(v); }
		void interval(const boost::posix_time::time_duration & v) override { (*this)(v); }
		void blob(const Blob & v) override { (*this)(v); }
		void null() override { }

		template <typename D, typename dummy = int>
		void operator()(const D &,
				typename boost::disable_if<std::is_convertible<D, T>, dummy>::type = 0) {
			BOOST_ERROR("Unexpected column type " << typeid(D).name());
		}

		template <typename D, typename dummy = int>
		void operator()(const D & v,
				typename boost::enable_if<std::is_convertible<D, T>, dummy>::type = 0) {
			BOOST_REQUIRE_EQUAL(expected, v);
		}

		const T & expected;
};

template<typename T>
void
TestCore::assertScalarValueHelper(DB::SelectCommand & sel, const T & t) const
{
	while (sel.fetch()) {
		assertColumnValueHelper(sel, 0, t);
	}
}

template<typename T>
void
TestCore::assertColumnValueHelper(DB::SelectCommand & sel, unsigned int col, const T & t) const
{
	Assert<T> a(t);
	sel[col].apply(a);
}

AdHocFormatter(BlobDbg, "Blob[length=%?, addr=%?]");
std::ostream &
operator<<(std::ostream & s, const DB::Blob b)
{
	BlobDbg::write(s, b.len, b.data);
	return s;
}

template void TestCore::assertScalarValueHelper<bool>(SelectCommand &, const bool &) const;
template void TestCore::assertScalarValueHelper<int64_t>(SelectCommand &, const int64_t &) const;
template void TestCore::assertScalarValueHelper<int>(SelectCommand &, const int &) const;
template void TestCore::assertScalarValueHelper<double>(SelectCommand &, const double &) const;
template void TestCore::assertScalarValueHelper<std::string>(SelectCommand &, const std::string &) const;
template void TestCore::assertScalarValueHelper<boost::posix_time::ptime>(SelectCommand &, const boost::posix_time::ptime &) const;
template void TestCore::assertScalarValueHelper<boost::posix_time::time_duration>(SelectCommand &, const boost::posix_time::time_duration &) const;
template void TestCore::assertScalarValueHelper<DB::Blob>(SelectCommand &, const DB::Blob &) const;

template void TestCore::assertColumnValueHelper<bool>(SelectCommand &, unsigned int, const bool &) const;
template void TestCore::assertColumnValueHelper<int>(SelectCommand &, unsigned int, const int &) const;
template void TestCore::assertColumnValueHelper<int64_t>(SelectCommand &, unsigned int, const int64_t &) const;
template void TestCore::assertColumnValueHelper<double>(SelectCommand &, unsigned int, const double &) const;
template void TestCore::assertColumnValueHelper<std::string>(SelectCommand &, unsigned int, const std::string &) const;
template void TestCore::assertColumnValueHelper<boost::posix_time::ptime>(SelectCommand &, unsigned int, const boost::posix_time::ptime &) const;
template void TestCore::assertColumnValueHelper<boost::posix_time::time_duration>(SelectCommand &, unsigned int, const boost::posix_time::time_duration &) const;
template void TestCore::assertColumnValueHelper<DB::Blob>(SelectCommand &, unsigned int, const DB::Blob &) const;

}

