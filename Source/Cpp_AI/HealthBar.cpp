#include "HealthBar.h"


void UHealthBar::NativeConstruct()
{
}

void UHealthBar::set_bar_value_percent(float const value)
{
	health_value->SetPercent(value);
}
