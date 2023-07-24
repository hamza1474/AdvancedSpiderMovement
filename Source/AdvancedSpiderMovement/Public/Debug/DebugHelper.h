#pragma once

namespace Debug
{
	static void Print(const FString& Msg, const FColor& Color = FColor::MakeRandomColor(), bool bOneFrame = false, int32 inKey = -1)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(inKey, bOneFrame ? 0.f : 10.f, Color, Msg);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg)
		}
	}
}
