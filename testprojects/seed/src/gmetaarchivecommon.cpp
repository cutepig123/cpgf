#include "gmetaarchivecommon.h"
#include "cpgf/gstdint.h"
#include "cpgf/gmetaapiutil.h"
#include "cpgf/gfixedtype.h"


namespace cpgf {


bool canSerializeItem(const GMetaArchiveConfig & config, IMetaItem * item)
{
	(void)config;

	GScopedInterface<IMetaAnnotation> annotation(item->getAnnotation(SerializationAnnotation));

	if(! annotation) {
	}
	else {
		GScopedInterface<IMetaAnnotationValue> annotationValue(annotation->getValue(SerializationAnnotationEnable));
		if(annotationValue) {
			if(! annotationValue->toBoolean()) {
				return false;
			}
		}
	}

	return true;
}

bool canSerializeObject(const GMetaArchiveConfig & config, IMetaClass * metaClass)
{
	return canSerializeItem(config, metaClass);
}

bool canSerializeField(const GMetaArchiveConfig & config, IMetaAccessible * accessible, IMetaClass * ownerClass)
{
	(void) ownerClass;

	if(! accessible->canGet()) {
		return false;
	}
	if(! accessible->canSet()) {
		return false;
	}

	return canSerializeItem(config, accessible);
}

bool canSerializeBaseClass(const GMetaArchiveConfig & config, IMetaClass * baseClass, IMetaClass * metaClass)
{
	(void)config;
	(void)metaClass;

	if(baseClass == NULL) {
		return false;
	}

	return true;
}

void serializeWriteObjectValue(IMetaArchiveWriter * archiveWriter, const char * name, void * instance, IMetaClass * metaClass)
{
	GMetaTypeData metaType = metaGetItemType(metaClass).getData();
	archiveWriter->writeObject(name, instance, &metaType, NULL);
}

void serializeWriteObjectPointer(IMetaArchiveWriter * archiveWriter, const char * name, void * instance, IMetaClass * metaClass)
{
	GMetaType metaType(metaGetItemType(metaClass));
	if(! metaType.isPointer()) {
		metaType.addPointer();
	}
	GMetaTypeData metaTypeData = metaType.getData();
	archiveWriter->writeObject(name, instance, &metaTypeData, NULL);
}

void serializeReadObject(IMetaArchiveReader * archiveReader, const char * name, void * instance, IMetaClass * metaClass)
{
	GMetaTypeData metaType = metaGetItemType(metaClass).getData();
	archiveReader->readObject(name, instance, &metaType, NULL);
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4800) // warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)
#endif

template <typename T>
GVariant doReadInteger(const void * address, unsigned int size)
{
	switch(size) {
		case 1:
			return (T)(*(typename GIfElse<IsSigned<T>::Result, GFixedTypeInt8::Signed, GFixedTypeInt8::Unsigned>::Result *)(address));

		case 2:
			return (T)(*(typename GIfElse<IsSigned<T>::Result, GFixedTypeInt16::Signed, GFixedTypeInt16::Unsigned>::Result *)(address));

		case 4:
			return (T)(*(typename GIfElse<IsSigned<T>::Result, GFixedTypeInt32::Signed, GFixedTypeInt32::Unsigned>::Result *)(address));

		case 8:
			return (T)(*(typename GIfElse<IsSigned<T>::Result, GFixedTypeInt64::Signed, GFixedTypeInt64::Unsigned>::Result *)(address));
	}

	GASSERT(false);

	return GVariant();
}

GVariant readFundamental(const void * address, const GMetaType & metaType)
{
	GVarTypeData typeData = metaType.getTypeData();
	unsigned int size = vtGetSize(typeData);
	if(vtIsReal(vtGetType(typeData))) {
		switch(size) {
			case 4:
				return *(GFixedTypeFloat32::Signed *)(address);

			case 8:
				return *(GFixedTypeFloat64::Signed *)(address);

			default:
				if(size == sizeof(long double)) { // long double has vary size on GCC...
					return *(GFixedTypeFloat80::Signed *)(address);
					break;
				}
				GASSERT(false);
				break;
		}
	}
	else {
		switch(vtGetType(typeData)) {
			case vtBool:
				return doReadInteger<bool>(address, size);

			case vtChar:
				return doReadInteger<char>(address, size);

			case vtWchar:
				return doReadInteger<wchar_t>(address, size);

			case vtSignedChar:
				return doReadInteger<signed char>(address, size);

			case vtUnsignedChar:
				return doReadInteger<unsigned char>(address, size);

			case vtSignedShort:
				return doReadInteger<signed short>(address, size);

			case vtUnsignedShort:
				return doReadInteger<unsigned short>(address, size);

			case vtSignedInt:
				return doReadInteger<signed int>(address, size);

			case vtUnsignedInt:
				return doReadInteger<unsigned int>(address, size);

			case vtSignedLong:
				return doReadInteger<signed long>(address, size);

			case vtUnsignedLong:
				return doReadInteger<unsigned long>(address, size);

			case vtSignedLongLong:
				return doReadInteger<signed long long>(address, size);

			case vtUnsignedLongLong:
				return doReadInteger<unsigned long long>(address, size);

			default:
				GASSERT(false);
				break;

		}
	}

	return GVariant();
}

template <typename T>
void doWriteInteger(void * address, unsigned int size, const GVariant & v)
{
	switch(size) {
		case 1:
			*(typename GIfElse<IsSigned<T>::Result, GFixedTypeInt8::Signed, GFixedTypeInt8::Unsigned>::Result *)(address) = fromVariant<typename GIfElse<IsSigned<T>::Result, GFixedTypeInt8::Signed, GFixedTypeInt8::Unsigned>::Result>(v);
			break;

		case 2:
			*(typename GIfElse<IsSigned<T>::Result, GFixedTypeInt16::Signed, GFixedTypeInt16::Unsigned>::Result *)(address) = fromVariant<typename GIfElse<IsSigned<T>::Result, GFixedTypeInt16::Signed, GFixedTypeInt16::Unsigned>::Result>(v);
			break;

		case 4:
			*(typename GIfElse<IsSigned<T>::Result, GFixedTypeInt32::Signed, GFixedTypeInt32::Unsigned>::Result *)(address) = fromVariant<typename GIfElse<IsSigned<T>::Result, GFixedTypeInt32::Signed, GFixedTypeInt32::Unsigned>::Result>(v);
			break;

		case 8:
			*(typename GIfElse<IsSigned<T>::Result, GFixedTypeInt64::Signed, GFixedTypeInt64::Unsigned>::Result *)(address) = fromVariant<typename GIfElse<IsSigned<T>::Result, GFixedTypeInt64::Signed, GFixedTypeInt64::Unsigned>::Result>(v);
			break;

		default:
			GASSERT(false);
	}
}

void writeFundamental(void * address, const GMetaType & metaType, const GVariant & v)
{
	GVarTypeData typeData = metaType.getTypeData();
	unsigned int size = vtGetSize(typeData);
	if(vtIsReal(vtGetType(typeData))) {
		switch(size) {
			case 4:
				*(GFixedTypeFloat32::Signed *)(address) = fromVariant<GFixedTypeFloat32::Signed>(v);
				break;

			case 8:
				*(GFixedTypeFloat64::Signed *)(address) = fromVariant<GFixedTypeFloat64::Signed>(v);
				break;

			default:
				if(size == sizeof(long double)) { // long double has vary size on GCC...
					*(GFixedTypeFloat80::Signed *)(address) = fromVariant<GFixedTypeFloat80::Signed>(v);
					break;
				}
				GASSERT(false);
				break;
		}
	}
	else {
		switch(vtGetType(typeData)) {
			case vtBool:
				doWriteInteger<bool>(address, size, v);
				break;

			case vtChar:
				doWriteInteger<char>(address, size, v);
				break;

			case vtWchar:
				doWriteInteger<wchar_t>(address, size, v);
				break;

			case vtSignedChar:
				doWriteInteger<signed char>(address, size, v);
				break;

			case vtUnsignedChar:
				doWriteInteger<unsigned char>(address, size, v);
				break;

			case vtSignedShort:
				doWriteInteger<signed short>(address, size, v);
				break;

			case vtUnsignedShort:
				doWriteInteger<unsigned short>(address, size, v);
				break;

			case vtSignedInt:
				doWriteInteger<signed int>(address, size, v);
				break;

			case vtUnsignedInt:
				doWriteInteger<unsigned int>(address, size, v);
				break;

			case vtSignedLong:
				doWriteInteger<signed long>(address, size, v);
				break;

			case vtUnsignedLong:
				doWriteInteger<unsigned long>(address, size, v);
				break;

			case vtSignedLongLong:
				doWriteInteger<signed long long>(address, size, v);
				break;

			case vtUnsignedLongLong:
				doWriteInteger<unsigned long long>(address, size, v);
				break;

			default:
				GASSERT(false);
				break;

		}
	}
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif


} // namespace cpgf



