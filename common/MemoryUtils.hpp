#pragma once

#include <memory>

/**
 * \brief dynamic_cast for unique_ptr
 * \tparam DST The destination pointer type, as would be for dynamic_cast<DST*>(SRC*)
 * \tparam SRC The source pointer type, as would be for dynamic_cast<DST*>(SRC*)
 * \param from The source pointer, as would be for dynamic_cast<DST*>(from)
 * \returns from casted from a unique_ptr to type SRC to a unique_ptr to type DST if DST is derived from SRC,
 *          otherwise null
 * \post If DST is derived from SRC, from is set to null
 *
 * This function is similar to dynamic_pointer_cast, but for unique_ptr instead of shared_ptr
 */
template <typename DST, typename SRC>
std::unique_ptr<DST> unique_dynamic_cast(std::unique_ptr<SRC>&& from)
{
	if (from == nullptr)
		return std::unique_ptr<DST>();
	
	DST* casted = dynamic_cast<DST*>(from.get());
	if (casted == nullptr)
		return std::unique_ptr<DST>();

	from.release();
	return std::unique_ptr<DST>(casted);
}
