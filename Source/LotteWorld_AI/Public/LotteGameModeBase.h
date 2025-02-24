// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/IHttpRequest.h"
#include "LotteGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class LOTTEWORLD_AI_API ALotteGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	ALotteGameModeBase();

public:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	void Audio2FaceCheck();
	
	// 오디오2페이스 초반 세팅 함수
	
	void CheckStatus();

	void ResponseCheckStatus(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void SetUSD();

	void ResponseSetUSD(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void SetActiveLiveLink();

	void ResponseSetActiveLiveLink(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void SetAutoGenerate();

	void ResponseSetAutoGenerate(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	UFUNCTION(BlueprintImplementableEvent)
	void EndAudio2FaceSetting();
	
	// 립싱크 관련 기능
	UPROPERTY()
	bool bCheckTalkStart = false;

	UFUNCTION()
	void TalkStartCheck();
	
	void SetPostOpenSetting();

	void ResponseSetPostOpenSetting(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void SetPostCloseSetting();

	void ResponseSetPostCloseSetting(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	UFUNCTION()
	void SetRoot();
	
	void SetRootTrack();

	void ResponseSetRootTrack(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void SetTrack();

	void ResponseSetTrack(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void GetRange();

	void ResponseGetRange(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void Play();

	void ResponsePlay(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void Pause();

	void ResponsePause(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void SetTrack0001();

	void ResponseSetTrack0001(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void Play0001();

	void ResponsePlay0001(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	// 저장 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString F_StateString = "";
	
	UPROPERTY()
	FString F_FileString = "";

	UPROPERTY()
	FString F_0001String = "";
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString F_GestureString = "";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString F_ScriptString = "";

	UPROPERTY()
	FString F_EventString = "";

	UPROPERTY()
	FString F_TimeString = "";

	UPROPERTY()
	FString F_RootString = "";

	// 오디오 길이 변수
	UPROPERTY()
	float audioRange = 0.0f;

	UPROPERTY()
	FTimerHandle AudioTimer;

	UFUNCTION(BlueprintImplementableEvent)
	void EndReadyWidget();
	
	UFUNCTION(BlueprintImplementableEvent)
	void ErrorWidgetOn();

	UFUNCTION(BlueprintImplementableEvent)
	void ErrorWidgetOff();
	
	// mqtt 받기 함수
	UFUNCTION(BlueprintCallable)
	void CallReceivedMQTT(FString Received);

	UFUNCTION(BlueprintImplementableEvent)
	void CallBackEndAssetAudio350();

	UFUNCTION(BlueprintImplementableEvent)
	void CallBackEndAssetAudio351();
	
	UFUNCTION(BlueprintImplementableEvent)
	void CallBackEndAssetAudio360Cake();

	UFUNCTION(BlueprintImplementableEvent)
	void CallBackEndAssetAudio360Hurray();
	
	UFUNCTION(BlueprintImplementableEvent)
	void CallBackEndAssetAudio706();
	
	UFUNCTION(BlueprintImplementableEvent)
	void CallBackEndBalloonAudio();
	
	UFUNCTION()
	void GetTime();

	UFUNCTION(BlueprintImplementableEvent)
	void OpenMapMorning();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OpenMapAfternoon();

	UFUNCTION(BlueprintImplementableEvent)
	void OpenMapDinner();

	UFUNCTION(BlueprintImplementableEvent)
	void EndPlayCam();

	UFUNCTION(BlueprintImplementableEvent)
	void State1_351();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void State1();

	UFUNCTION(BlueprintImplementableEvent)
	void state2();
	
	UFUNCTION()
	void State311();
	
	UFUNCTION()
	void State330();

	UFUNCTION()
	void State331();
	
	UFUNCTION()
	void State340();

	UFUNCTION()
	void State360();

	UFUNCTION(BlueprintImplementableEvent)
	void StartCake();

	UFUNCTION(BlueprintImplementableEvent)
	void StartHurray();
	
	UFUNCTION()
	void State400();

	UFUNCTION(BlueprintCallable)
	void State600_1();

	UFUNCTION(BlueprintCallable)
	void State600_Fireworks();
	
	UFUNCTION(BlueprintCallable)
	void State600_2();

	UFUNCTION(BlueprintCallable)
	void State600_3();
	
	UFUNCTION(BlueprintCallable)
	void State600_Result();

	UFUNCTION()
	void State702(int32 idx);

	UFUNCTION()
	void State706(FString result);
	
	UPROPERTY()
	int32 Retime = 0;
	
	UFUNCTION(BlueprintImplementableEvent)
	void CallBackErrorA2F();

	UFUNCTION(BlueprintImplementableEvent)
	void NotExistAudio();
	
	UFUNCTION(BlueprintCallable)
	void StartBalloonSpeak();

	UFUNCTION(BlueprintImplementableEvent)
	void StartBoring();

	UFUNCTION()
	void State350(FString Sub_Category, FString Location, FString Entity, FString Asset);
	
	UFUNCTION(BlueprintCallable)
	void StartAsset();

	UFUNCTION()
	void Set0001();
	
	UFUNCTION(BlueprintImplementableEvent)
	void PlayInfoMontage();
	
	UFUNCTION(BlueprintCallable)
	void SetSpeechBalloon();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BalloonIdx;
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetBalloon1();

	UFUNCTION(BlueprintImplementableEvent)
	void SetBalloon2();

	UFUNCTION()
	void Branch_1F_Attraction_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_1F_Dining_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_1F_Gift_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_1F_Play_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_Museum_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_1F_Amenities_Asset(FString Entity);

	UFUNCTION()
	void Branch_2F_Attraction_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_2F_Dining_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_2F_Gift_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_2F_Play_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_2F_Amenities_Asset(FString Entity);

	UFUNCTION()
	void Branch_3F_Attraction_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_3F_Dining_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_3F_Amenities_Asset(FString Entity);

	UFUNCTION()
	void Branch_4F_Attraction_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_4F_Dining_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_4F_Gift_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_4F_Play_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_4F_Amenities_Asset(FString Entity);

	UFUNCTION()
	void Branch_Magic_Attraction_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_Magic_Dining_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_Magic_Gift_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_Magic_Amenities_Asset(FString Entity);
	
	UFUNCTION()
	void Branch_Under_Attraction_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_Under_Dining_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_Under_Gift_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_Under_Play_Asset(FString Entity, FString Asset);

	UFUNCTION()
	void Branch_Under_Amenities_Asset(FString Entity);

	UFUNCTION(BlueprintImplementableEvent)
	void Info_Attraction(const FString& Name, const int32& Asset);

	UFUNCTION(BlueprintImplementableEvent)
	void Info_Dining(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Info_Gift(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_1F_Attraction(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_1F_Dining(const FString& Name);
	
	UFUNCTION(BlueprintImplementableEvent)
	void Map_1F_Gift(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_2F_Attraction(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_2F_Dining(const FString& Name);
	
	UFUNCTION(BlueprintImplementableEvent)
	void Map_2F_Gift(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_3F_Attraction(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_3F_Dining(const FString& Name);
	
	UFUNCTION(BlueprintImplementableEvent)
	void Map_3F_Gift(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_4F_Attraction(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_4F_Dining(const FString& Name);
	
	UFUNCTION(BlueprintImplementableEvent)
	void Map_4F_Gift(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_Magic_Attraction(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_Magic_Dining(const FString& Name);
	
	UFUNCTION(BlueprintImplementableEvent)
	void Map_Magic_Gift(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_Under_Attraction(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_Under_Dining(const FString& Name);
	
	UFUNCTION(BlueprintImplementableEvent)
	void Map_Under_Gift(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_1F_Amenities(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_2F_Amenities(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_3F_Amenities(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_4F_Amenities(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_Magic_Amenities(const FString& Name);

	UFUNCTION(BlueprintImplementableEvent)
	void Map_Under_Amenities(const FString& Name);

	UFUNCTION()
	void LotteTime351(FString sub_category);

	UFUNCTION(BlueprintImplementableEvent)
	void LotteTimeOn(const FString& sub_category, const FString& date, const FString& day, const FString& start, const FString& end);
	
	UFUNCTION()
	void PerformListTime351(FString entity);

	UFUNCTION(BlueprintImplementableEvent)
	void PerformListOn(const FString& entity, const TArray<FString>& StringList);
	
	TArray<TSharedPtr<FJsonValue>> PerformList;

	UPROPERTY()
	TArray<FString> PerformListStrings;

	UPROPERTY()
	TArray<FString> times;

	UFUNCTION()
	void TimeCheckFunc();

	UFUNCTION(BlueprintImplementableEvent)
	void PerformInfo(const TArray<FString>& timeList, const FString& name);

	UFUNCTION(BlueprintImplementableEvent)
	void PerformMap(const FString& name);
	
	UFUNCTION()
	void PerformAsset351(FString entity, FString asset, FString location);

	UFUNCTION(BlueprintImplementableEvent)
	void CallBackFSM1();
	
	UFUNCTION(BlueprintCallable)
	void TestAudio(const FString& string, const FString& audio, const FString& sub, const FString& loc, const FString& entity, const FString& asset);
};