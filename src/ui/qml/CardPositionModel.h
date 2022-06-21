/*!
 * \brief Model for card positioning animations.
 *
 * \copyright Copyright (c) 2020-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "CardPosition.h"

#include <QObject>
#include <QTimer>
#include <QVector>

class test_CardPositionModel;

namespace governikus
{

class CardPositionModel
	: public QObject
{
	Q_OBJECT
	friend class ::test_CardPositionModel;

	Q_PROPERTY(int count READ getCardPositionCount CONSTANT)
	Q_PROPERTY(int cyclingClock READ getCyclingClock WRITE setCyclingClock NOTIFY fireCyclingClockChanged)
	Q_PROPERTY(bool running READ getIsRunning WRITE setIsRunning NOTIFY fireIsRunningChanged)

	private:
		int mCyclingClock;
		int mCurrentIndex;
		QTimer mCyclingTimer;
		const QVector<CardPosition> mCardPositions;

		void startTimer();
		void stopTimer();

	public:
		CardPositionModel();

		int getCyclingClock() const;
		void setCyclingClock(int pCyclingClock);
		Q_INVOKABLE QVariant getCardPosition() const;
		bool getIsRunning() const;
		void setIsRunning(bool pRunning);
		int getCardPositionCount() const;

	Q_SIGNALS:
		void fireCyclingClockChanged();
		void fireCardPositionChanged();
		void fireIsRunningChanged();

	private Q_SLOTS:
		void onTimerTimeout();
};


} // namespace governikus
