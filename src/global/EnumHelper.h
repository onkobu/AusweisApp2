/*!
 * \brief Helper class to provide a QMetaObject handler for enumerations.
 *
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include <QDebug>
#include <QMetaEnum>
#include <type_traits>


namespace governikus
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define defineQHash(enumName)\
	inline size_t qHash(enumName pKey, size_t pSeed)\
	{\
		return ::qHash(static_cast<std::underlying_type<enumName>::type>(pKey), pSeed);\
	}
#else
#define defineQHash(enumName)\
	inline uint qHash(enumName pKey, uint pSeed)\
	{\
		return ::qHash(static_cast<std::underlying_type<enumName>::type>(pKey), pSeed);\
	}
#endif

#define defineEnumOperators(enumName)\
	inline QDebug operator<<(QDebug pDbg, enumName pType)\
	{\
		QDebugStateSaver saver(pDbg);\
		return pDbg.noquote() << Enum<enumName>::getName(pType);\
	}\
\
	inline QString& operator+=(QString & pStr, enumName pType)\
	{\
		pStr += Enum<enumName>::getName(pType);\
		return pStr;\
	}\
\
	inline QString operator+(const QString& pStr, enumName pType)\
	{\
		return pStr + Enum<enumName>::getName(pType);\
	}\
\
	inline QString operator+(enumName pType, const QString& pStr)\
	{\
		return Enum<enumName>::getName(pType) + pStr;\
	}\
\
	inline bool operator==(std::underlying_type<enumName>::type pType, enumName pName)\
	{\
		return static_cast<std::underlying_type<enumName>::type>(pName) == pType;\
	}\
	inline bool operator!=(std::underlying_type<enumName>::type pType, enumName pName)\
	{\
		return !(pType == pName);\
	}\
	defineQHash(enumName)


#define defineTypedEnumType(enumName, enumType, ...)\
	class Enum##enumName\
	{\
		Q_GADGET\
		private:\
			Enum##enumName();\
			Q_DISABLE_COPY(Enum##enumName)\
\
		public:\
			enum class enumName : enumType\
			{\
				__VA_ARGS__\
			};\
\
			Q_ENUM(enumName)\
	};\
\
	using enumName = Enum##enumName::enumName;\
\
	defineEnumOperators(enumName)


#define defineEnumType(enumName, ...) defineTypedEnumType(enumName, int, __VA_ARGS__)


template<typename EnumTypeT> class Enum
{
	using EnumBaseTypeT = typename std::underlying_type<EnumTypeT>::type;

	private:
		Enum() = delete;
		Q_DISABLE_COPY(Enum)

	public:
		static inline QMetaEnum getQtEnumMetaEnum()
		{
			return QMetaEnum::fromType<EnumTypeT>();
		}


		static QLatin1String getName()
		{
			return QLatin1String(getQtEnumMetaEnum().name());
		}


		static QLatin1String getName(EnumTypeT pType)
		{
			const int value = static_cast<int>(pType);
			const char* const name = getQtEnumMetaEnum().valueToKey(value);
			if (Q_UNLIKELY(name == nullptr))
			{
				qCritical().noquote().nospace() << "CRITICAL CONVERSION MISMATCH: UNKNOWN 0x" << QString::number(value, 16);
				return QLatin1String();
			}

			return QLatin1String(name);
		}


		static int getCount()
		{
			return getQtEnumMetaEnum().keyCount();
		}


		static QVector<EnumTypeT> getList()
		{
			QVector<EnumTypeT> list;

			const QMetaEnum metaEnum = getQtEnumMetaEnum();
			list.reserve(metaEnum.keyCount());
			for (int i = 0; i < metaEnum.keyCount(); ++i)
			{
				list << static_cast<EnumTypeT>(metaEnum.value(i));
			}

			return list;
		}


		static EnumTypeT fromString(const char* const pValue, EnumTypeT pDefault)
		{
			bool ok = false;
			int key = getQtEnumMetaEnum().keyToValue(pValue, &ok);
			if (ok)
			{
				return static_cast<EnumTypeT>(key);
			}
			return pDefault;
		}


		static EnumTypeT fromString(const QString& pValue, EnumTypeT pDefaultType)
		{
			return fromString(pValue.toUtf8().constData(), pDefaultType);
		}


		static bool isValue(int pValue)
		{
			return getQtEnumMetaEnum().valueToKey(pValue) != nullptr;
		}


		static bool isValue(uchar pValue)
		{
			return isValue(static_cast<int>(pValue));
		}


		static bool isValue(char pValue)
		{
			return isValue(static_cast<uchar>(pValue));
		}


		static EnumBaseTypeT getValue(EnumTypeT pType)
		{
			return static_cast<EnumBaseTypeT>(pType);
		}


};


template<typename T> inline QLatin1String getEnumName(T pType)
{
	return Enum<T>::getName(pType);
}


} // namespace governikus
