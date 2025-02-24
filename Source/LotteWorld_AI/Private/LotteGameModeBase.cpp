// Fill out your copyright notice in the Description page of Project Settings.


#include "LotteGameModeBase.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

ALotteGameModeBase::ALotteGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
}

void ALotteGameModeBase::BeginPlay()
{
	Super::BeginPlay();

}

void ALotteGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
}

void ALotteGameModeBase::Audio2FaceCheck()
{
	CheckStatus();
}

void ALotteGameModeBase::CheckStatus()
{
	// 체크 횟수 저장
	Retime = Retime++;
	
	// audio2face status check
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponseCheckStatus);
	Request->SetURL("http://localhost:8011/status");
	Request->SetVerb("Get");
	Request->SetHeader("accept", "application/json");
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponseCheckStatus(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	if(Response == nullptr)
	{
		// audio2face 실행 안되어 있을때
		UE_LOG(LogTemp, Warning, TEXT("audio2face error"));

		return;
	}
	
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);
	
	UE_LOG(LogTemp, Warning, TEXT("audio2face status %s"), *Response->GetContentAsString());
	
	if(bConnectedSuccessfully != false)
	{
		SetUSD();
	}
	else
	{
		// 3번 이상 못 찾으면 에러 푸쉬
		if(Retime < 3)
		{
			// 상태 확인 재시도
			FTimerHandle time;
			GetWorldTimerManager().SetTimer(time, this, &ALotteGameModeBase::CheckStatus, 10.0f, false);
		}
		else
		{
			CallBackErrorA2F();
		}
	}
}

void ALotteGameModeBase::SetUSD()
{
	// usd setting
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("file_name", "omniverse://localhost/Users/kiosk/lotte.usd");
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponseSetUSD);
	Request->SetURL("http://localhost:8011/A2F/USD/Load");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponseSetUSD(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);
	
	UE_LOG(LogTemp, Warning, TEXT("ResponseSetUSD %s"), *Response->GetContentAsString());
	
	if(bConnectedSuccessfully != false)
	{
		// 끝나면 다음 과정으로
		FTimerHandle time;
		GetWorldTimerManager().SetTimer(time, this, &ALotteGameModeBase::SetActiveLiveLink, 1.0f, false);
	}
	else
	{
		// 오류 mqtt 쏘기
		UE_LOG(LogTemp, Display, TEXT("audio2face error~!!!! mqtt 9 push"));
		
		CallBackErrorA2F();
	}
}

void ALotteGameModeBase::SetActiveLiveLink()
{
	// post active live link setting
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("node_path", "/World/audio2face/StreamLivelink");
	RequestObj->SetStringField("value", "true");

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponseSetActiveLiveLink);
	Request->SetURL("http://localhost:8011/A2F/Exporter/ActivateStreamLivelink");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponseSetActiveLiveLink(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Warning, TEXT("ResponseActiveLiveLink %s"), *Response->GetContentAsString());

	if(bConnectedSuccessfully != false)
	{
		// 끝나면 다음 과정으로
		FTimerHandle time;
		GetWorldTimerManager().SetTimer(time, this, &ALotteGameModeBase::SetAutoGenerate, 2.0f, false);
	}
	else
	{
		// 오류 mqtt 쏘기
		UE_LOG(LogTemp, Display, TEXT("audio2face error~!!!! mqtt 9 push"));
		
		CallBackErrorA2F();
	}
}

void ALotteGameModeBase::SetAutoGenerate()
{
	// auto generate setting
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("a2f_instance", "/World/audio2face/CoreFullface");
	RequestObj->SetStringField("enable", "true");

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponseSetAutoGenerate);
	Request->SetURL("http://localhost:8011/A2F/A2E/EnableAutoGenerateOnTrackChange");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponseSetAutoGenerate(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Warning, TEXT("ResponseAutoGenerate %s"), *Response->GetContentAsString());

	if(bConnectedSuccessfully != false)
	{
		// 끝나면 다음 과정으로
		// 시작 세팅 끝
		UE_LOG(LogTemp, Warning, TEXT("start setting end"));

		EndAudio2FaceSetting();
	}
	else
	{
		// 오류 mqtt 쏘기
		UE_LOG(LogTemp, Display, TEXT("audio2face error~!!!! mqtt 9 push"));
		
		CallBackErrorA2F();
	}
}

// 말하기 전 입 모양 체크 함수
void ALotteGameModeBase::TalkStartCheck()
{
	// 말하려는거면
	if(bCheckTalkStart == false)
	{
		// 입 모양 오므리기
		SetPostCloseSetting();
		
		bCheckTalkStart = true;
	}
	// 말끝난거면
	else
	{
		// 입 모양 벌리기
		SetPostOpenSetting();

		bCheckTalkStart = false;
	}

	UE_LOG(LogTemp, Warning, TEXT("bchecktalkstart : %s"), bCheckTalkStart ? TEXT("True") : TEXT("False"));
}

void ALotteGameModeBase::SetPostOpenSetting()
{
	// postprocessing setting
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("a2f_instance", "/World/audio2face/CoreFullface");
	
	TSharedRef<FJsonObject> ValueObject = MakeShared<FJsonObject>();

	ValueObject->SetNumberField("lip_open_offset", 0.19f);
	
	RequestObj->SetObjectField("settings", ValueObject);
	
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponseSetPostOpenSetting);
	Request->SetURL("http://localhost:8011/A2F/POST/SetSettings");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponseSetPostOpenSetting(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Warning, TEXT("ResponseSetPostProcessing %s"), *Response->GetContentAsString());

	if(bConnectedSuccessfully != false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Set post processting open success"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Set post processting open error"));

		CallBackErrorA2F();
	}
}

void ALotteGameModeBase::SetPostCloseSetting()
{
	// postprocessing setting
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("a2f_instance", "/World/audio2face/CoreFullface");
	
	TSharedRef<FJsonObject> ValueObject = MakeShared<FJsonObject>();
	
	ValueObject->SetNumberField("lip_open_offset", 0.0f);
	
	RequestObj->SetObjectField("settings", ValueObject);
	
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponseSetPostCloseSetting);
	Request->SetURL("http://localhost:8011/A2F/POST/SetSettings");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponseSetPostCloseSetting(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Warning, TEXT("ResponseSetPostProcessing %s"), *Response->GetContentAsString());

	if(bConnectedSuccessfully != false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Set post processting close success"));

		FTimerHandle time;
		GetWorldTimerManager().SetTimer(time, this, &ALotteGameModeBase::SetRoot, 0.1, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Set post processting close error"));

		CallBackErrorA2F();
	}
}

void ALotteGameModeBase::SetRoot()
{
	// 상대 경로 지정
	const FString pro_Dir = FPaths::ProjectDir();
	const FString dir_1 = FPaths::GetPath(pro_Dir);
	const FString dir_2 = FPaths::GetPath(dir_1);
	const FString check_1 = FPaths::ConvertRelativePathToFull(dir_2);
	const FString dir_3 = FPaths::GetPath(check_1);
	const FString dir_4 = FPaths::GetPath(dir_3);
	const FString audio = TEXT("/audio");
	const FString dir_5 = FPaths::Combine(dir_4, audio);
	
	int32 idx = F_FileString.Len();
	FString name = F_FileString.Left(2);

	// 파일 이름에 따라서 audio2face 디렉토리 변경하기
	if(name == TEXT("AA"))
	{
		if(idx == 10)
		{
			FString after = TEXT("/0/AA");

			FString final_Dir = FPaths::Combine(dir_5, after);

			if(final_Dir != F_RootString)
			{
				F_RootString = final_Dir;
			}

			SetRootTrack();
		}
		else
		{
			FString num = F_FileString.Mid(2,5);
			int32 number = FCString::Atoi(*num);
			int32 result = number/10000;
			FString back1 = FString::FromInt(result);
			FString front1 = TEXT("/");
			FString back2 = FPaths::Combine(front1, back1);
			FString back3 = TEXT("/AA");
			FString back4 = FPaths::Combine(back2, back3);
			FString final_Dir = FPaths::Combine(dir_5, back4);
		
			if(final_Dir != F_RootString)
			{
				F_RootString = final_Dir;
			}
		
			SetRootTrack();
		}
	}
	else if(name == TEXT("AB"))
	{
		if(idx == 10)
		{
			FString after = TEXT("/0/AB");

			FString final_Dir = FPaths::Combine(dir_5, after);

			if(final_Dir != F_RootString)
			{
				F_RootString = final_Dir;
			}

			SetRootTrack();
		}
		else
		{
			FString num = F_FileString.Mid(2,5);
			int32 number = FCString::Atoi(*num);
			int32 result = number/10000;
			FString back1 = FString::FromInt(result);
			FString front1 = TEXT("/");
			FString back2 = FPaths::Combine(front1, back1);
			FString back3 = TEXT("/AB");
			FString back4 = FPaths::Combine(back2, back3);
			FString final_Dir = FPaths::Combine(dir_5, back4);
		
			if(final_Dir != F_RootString)
			{
				F_RootString = final_Dir;
			}
		
			SetRootTrack();
		}
	}
	else if(name == TEXT("AC"))
	{
		if(idx == 10)
		{
			FString after = TEXT("/0/AC");

			FString final_Dir = FPaths::Combine(dir_5, after);

			if(final_Dir != F_RootString)
			{
				F_RootString = final_Dir;
			}

			SetRootTrack();
		}
		else
		{
			FString num = F_FileString.Mid(2,5);
			int32 number = FCString::Atoi(*num);
			int32 result = number/10000;
			FString back1 = FString::FromInt(result);
			FString front1 = TEXT("/");
			FString back2 = FPaths::Combine(front1, back1);
			FString back3 = TEXT("/AC");
			FString back4 = FPaths::Combine(back2, back3);
			FString final_Dir = FPaths::Combine(dir_5, back4);
		
			if(final_Dir != F_RootString)
			{
				F_RootString = final_Dir;
			}
		
			SetRootTrack();
		}
	}
	else
	{
		FString after = TEXT("/default");

		FString final_Dir = FPaths::Combine(dir_5, after);

		if(final_Dir != F_RootString)
		{
			F_RootString = final_Dir;
		}
		
		SetRootTrack();
	}
}

void ALotteGameModeBase::SetRootTrack()
{
	// root path setting
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("a2f_player", "/World/audio2face/Player");
	RequestObj->SetStringField("dir_path", F_RootString);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponseSetRootTrack);
	Request->SetURL("http://localhost:8011/A2F/Player/SetRootPath");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponseSetRootTrack(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Warning, TEXT("ResponseRootPath %s"), *Response->GetContentAsString());

	if(bConnectedSuccessfully != false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Set post processting close success"));

		FTimerHandle time;
		GetWorldTimerManager().SetTimer(time, this, &ALotteGameModeBase::SetTrack, 0.1, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Set post processting close error"));

		CallBackErrorA2F();
	}
}

void ALotteGameModeBase::SetTrack()
{
	// audio track setting
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	
	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("a2f_player", "/World/audio2face/Player");
	RequestObj->SetStringField("file_name", F_FileString);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponseSetTrack);
	Request->SetURL("http://localhost:8011/A2F/Player/SetTrack");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponseSetTrack(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Warning, TEXT("ResponseSetTrack %s"), *Response->GetContentAsString());

	if(bConnectedSuccessfully != false)
	{
		// 끝나면 다음 과정으로
		GetRange();
	}
	else
	{
		// 오디오 없음

		UE_LOG(LogTemp, Display, TEXT("audio emty or audio2face error~!!!! mqtt 9 push"));

		CallBackErrorA2F();
		
		NotExistAudio();
	}
}

void ALotteGameModeBase::GetRange()
{
	// get audio range
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	
	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("a2f_player", "/World/audio2face/Player");
	
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponseGetRange);
	Request->SetURL("http://localhost:8011/A2F/Player/GetRange");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponseGetRange(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Warning, TEXT("ResponseGetRange %s"), *Response->GetContentAsString());

	// json 구조체 읽어서 오디오 길이 float에 담기
	TSharedPtr<FJsonObject> object = ResponseObj->GetObjectField(TEXT("result"));
	TArray<TSharedPtr<FJsonValue>> objectArray = object->GetArrayField(TEXT("default"));

	if(objectArray[1] != nullptr)
	{
		auto idx = objectArray[1]->AsNumber();

		audioRange = Chaos::ConvertDoubleToFloat(idx);
	}
	else
	{
		audioRange = 0;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("ResponseGetRange %f"), audioRange);

	if(bConnectedSuccessfully != false)
	{
		// 끝나면 다음 과정으로
		Play();
	}
	else
	{
		CallBackErrorA2F();
	}
}

void ALotteGameModeBase::Play()
{
	// post audio play
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("a2f_player", "/World/audio2face/Player");

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponsePlay);
	Request->SetURL("http://localhost:8011/A2F/Player/Play");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponsePlay(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Warning, TEXT("ResponsePlay %s"), *Response->GetContentAsString());
	
	if(bConnectedSuccessfully != false)
	{
		FTimerDelegate TimerDelegate;
		
		TimerDelegate.BindLambda([this]()
		{
			if(F_StateString == "350")
			{
				CallBackEndAssetAudio350();
			}
			else if(F_StateString == "351")
			{
				CallBackEndAssetAudio351();
			}
			else if (F_StateString == "360")
			{
				if(F_EventString == TEXT("기념일"))
				{
					CallBackEndAssetAudio360Cake();
				}
				else
				{
					CallBackEndAssetAudio360Hurray();
				}
			}
			else if (F_StateString == "706")
			{
				CallBackEndAssetAudio706();
			}
			else if(F_StateString != "600_Fire")
			{
				CallBackEndBalloonAudio();
			}

			TalkStartCheck();
		});
	
		GetWorldTimerManager().SetTimer(AudioTimer, TimerDelegate, audioRange, false);
	}
	else
	{
		CallBackErrorA2F();
	}
}

void ALotteGameModeBase::Pause()
{
	// post audio pause
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("a2f_player", "/World/audio2face/Player");

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponsePause);
	Request->SetURL("http://localhost:8011/A2F/Player/Pause");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponsePause(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Warning, TEXT("ResponsePause %s"), *Response->GetContentAsString());

	if(bConnectedSuccessfully != false)
	{
		// 게임 인스턴스 사운드 정지
		
	}
}

void ALotteGameModeBase::SetTrack0001()
{
	// audio track setting
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	
	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("a2f_player", "/World/audio2face/Player");
	RequestObj->SetStringField("file_name", F_0001String);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponseSetTrack0001);
	Request->SetURL("http://localhost:8011/A2F/Player/SetTrack");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponseSetTrack0001(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Warning, TEXT("ResponseSetTrack %s"), *Response->GetContentAsString());

	if(bConnectedSuccessfully != false)
	{
		// 끝나면 다음 과정으로
		Play0001();
	}
	else
	{
		// 오디오 없음
		UE_LOG(LogTemp, Display, TEXT("audio emty or audio2face error~!!!! mqtt 9 push"));

		CallBackErrorA2F();
	}
}

void ALotteGameModeBase::Play0001()
{
	// post audio play
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("a2f_player", "/World/audio2face/Player");

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ALotteGameModeBase::ResponsePlay0001);
	Request->SetURL("http://localhost:8011/A2F/Player/Play");
	Request->SetVerb("Post");
	Request->SetHeader("accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALotteGameModeBase::ResponsePlay0001(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Warning, TEXT("ResponsePlay %s"), *Response->GetContentAsString());
	
	if(bConnectedSuccessfully != false)
	{
		UE_LOG(LogTemp, Warning, TEXT("play 0001 success"));
	}
	else
	{
		CallBackErrorA2F();
	}
}

void ALotteGameModeBase::CallReceivedMQTT(FString Received)
{
	// mqtt 통신 변수에 동기화
	TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(Received);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	FJsonSerializer::Deserialize(reader, jsonObject);

	// state 분별
	FString stateString = jsonObject->GetStringField(TEXT("state"));
	
	UE_LOG(LogTemp, Warning, TEXT("state : %s"), *stateString);
		
	TSharedPtr<FJsonObject> object = jsonObject->GetObjectField(TEXT("payload"));

	// 시작 state
	if(stateString == "1")
	{
		// ui 사라지게 하기
		if(F_StateString == "311" || F_StateString == "330" || F_StateString == "331" || F_StateString == "340" || F_StateString == "350" || F_StateString == "702" || F_StateString == "706")
		{
			State1();
		}
		// 공연 ui 사라지게 하기
		else if(F_StateString == "351")
		{
			State1_351();
		}
		// 에러 ui 사라지게 하기
		else if(F_StateString == "9")
		{
			ErrorWidgetOff();
		}
		
		F_StateString = stateString;
	}
	else if(stateString == "2")
	{
		EndReadyWidget();
		
		state2();
	}
	// 에러 ui 나오게 하기
	else if(stateString == "9")
	{
		F_StateString = stateString;

		ErrorWidgetOn();
	}
	// 필러 랜덤 발화
	else if(stateString == "311")
	{
		EndReadyWidget();
		
		F_StateString = stateString;

		State311();
	}
	// 음성 인식 발화
	else if(stateString == "330")
	{
		EndReadyWidget();
		
		F_StateString = stateString;

		State330();
	}
	// 대화 추천 발화
	else if(stateString == "331")
	{
		EndReadyWidget();
		
		F_StateString = stateString;

		State331();
	}
	// 시나리오 발화
	else if(stateString == "340")
	{
		EndReadyWidget();
		
		FString gesture_String = object->GetStringField(TEXT("gesture"));
		FString script_String = object->GetStringField(TEXT("script"));
		FString filename_String = object->GetStringField(TEXT("filename"));

		UE_LOG(LogTemp, Warning, TEXT("script : %s"), *script_String);
	
		F_StateString = stateString;
		F_ScriptString = script_String;
		F_FileString = filename_String;
		F_GestureString = gesture_String;
		
		State340();
	}
	// 정보 발화
	else if(stateString == "350")
	{
		EndReadyWidget();
		
		FString sub_category_String = object->GetStringField(TEXT("sub_category"));
		FString location_String = object->GetStringField(TEXT("location"));
		FString entity_String = object->GetStringField(TEXT("entity"));
		FString asset_String = object->GetStringField(TEXT("asset"));
		FString a_script_String = object->GetStringField(TEXT("answer_script"));
		FString filename_String = object->GetStringField(TEXT("filename"));
		
		F_StateString = stateString;
		F_FileString = filename_String;
		
		State350(sub_category_String, location_String, entity_String, asset_String);
	}
	// 실시간 퍼레이트 발화
	else if(stateString == "351")
	{
		EndReadyWidget();
		
		FString sub_category_String = object->GetStringField(TEXT("sub_category"));
		FString entity_String = object->GetStringField(TEXT("entity"));
		FString asset_String = object->GetStringField(TEXT("asset"));
		FString filename_String = object->GetStringField(TEXT("filename"));
		
		F_StateString = stateString;
		F_FileString = filename_String;
		
		if(sub_category_String == TEXT("롯데월드"))
		{
			FString time = object->GetStringField(TEXT("payload"));

			F_TimeString = time;
			
			LotteTime351(sub_category_String);
		}
		else
		{
			if(entity_String == TEXT("오늘의 공연"))
			{
				PerformList.Empty();
				
				TArray<TSharedPtr<FJsonValue>> timeList = object->GetArrayField(TEXT("payload"));

				PerformList = timeList;

				PerformListTime351(entity_String);
			}
			// 공연 이름
			else
			{
				FString time = object->GetStringField(TEXT("payload"));
				FString location = object->GetStringField(TEXT("location"));

				F_TimeString = time;
					
				PerformAsset351(entity_String, asset_String, location);
			}
		}
	}
	// 이벤트 발화
	else if(stateString == "360")
	{
		EndReadyWidget();
		
		FString entity_String = object->GetStringField(TEXT("entity"));
		FString filename_String = object->GetStringField(TEXT("filename"));

		F_StateString = stateString;
		F_FileString = filename_String;
		F_EventString = entity_String;
		
		State360();
	}
	// 대기 중 애니메이션
	else if(stateString == "400")
	{
		State400();
	}
	// 셀카 
	else if(stateString == "600")
	{
		EndReadyWidget();
		
		State600_1();
	}
	// 어트랙션 추천 질문 발화
	else if(stateString == "702")
	{
		EndReadyWidget();
		
		int32 index_int = object->GetIntegerField(TEXT("index"));
		FString script_String = object->GetStringField(TEXT("question"));

		F_StateString = stateString;
		F_ScriptString = script_String;

		State702(index_int);
	}
	// 어트랙션 결과 발화
	else if(stateString == "706")
	{
		FString result_String = object->GetStringField(TEXT("attraction"));

		F_StateString = stateString;

		State706(result_String);
	}
}

void ALotteGameModeBase::GetTime()
{
	FDateTime currentTime = FDateTime::Now();

	int32 hour = currentTime.GetHour();

	UE_LOG(LogTemp, Warning, TEXT("hour is %d"), hour);

	if(hour >= 6 && hour < 12)
	{
		OpenMapMorning();
	}
	else if(hour >= 12 && hour < 18)
	{
		OpenMapAfternoon();
	}
	else if(hour >= 18 || hour < 6)
	{
		OpenMapDinner();
	}
}

void ALotteGameModeBase::State311()
{
	int32 num = FMath::RandRange(1,8);

	FString front = "S000";
	FString number = front.Append(FString::FromInt(num));
	FString full = number.Append(".wav");
	
	F_FileString = full;

	if(num == 1)
	{
		F_ScriptString = TEXT("잠깐만 기다려줘!");;
	}
	else if(num == 2)
	{
		F_ScriptString = TEXT("음, 그건 말이야!");
	}
	else if(num == 3)
	{
		F_ScriptString = TEXT("음, 잠깐 생각해 볼게!");
	}
	else if(num == 4)
	{
		F_ScriptString = TEXT("생각할 시간이 필요해!");
	}
	else if(num == 5)
	{
		F_ScriptString = TEXT("음, 잘 생각해 볼게!");
	}
	else if(num == 6)
	{
		F_ScriptString = TEXT("금방 도와줄게!");
	}
	else if(num == 7)
	{
		F_ScriptString = TEXT("잠깐만!");
	}
	else if(num == 8)
	{
		F_ScriptString = TEXT("조금만 기다려줘!");
	}
	
	SetSpeechBalloon();
}

void ALotteGameModeBase::State330()
{
	int32 num = FMath::RandRange(9,18);

	if(num == 9)
	{
		FString front = "S000";
		FString number = front.Append(FString::FromInt(num));
		FString full = number.Append(".wav");

		F_FileString = full;
		
		F_ScriptString = TEXT("미안! 질문을 잘 못 들었어! 다시 한번 얘기해줄 수 있니?");
	}
	else
	{
		FString front = "S00";
		FString number = front.Append(FString::FromInt(num));
		FString full = number.Append(".wav");

		F_FileString = full;

		if(num == 10)
		{
			F_ScriptString = TEXT("음, 내가 잘못 알아들었나 봐! 다시 말해줄 수 있니?");
		}
		else if(num == 11)
		{
			F_ScriptString = TEXT("음, 그 질문이 조금 헷갈리네! 다시 설명해 줄 수 있니?");
		}
		else if(num == 12)
		{
			F_ScriptString = TEXT("음, 질문이 잘 이해가 안 됐어! 한 번 더 말해줄 수 있을까?");
		}
		else if(num == 13)
		{
			F_ScriptString = TEXT("미안! 잘 못 알아들었어! 다시 한번 얘기해주라!");
		}
		else if(num == 14)
		{
			F_ScriptString = TEXT("음, 내가 잘못 들었나 봐! 다시 한번 알려줘!");
		}
		else if(num == 15)
		{
			F_ScriptString = TEXT("음, 조금 더 가까이 와서 물어봐 줄 수 있겠니?");
		}
		else if(num == 16)
		{
			F_ScriptString = TEXT("음, 질문을 제대로 못 들은 것 같아! 다시 한번 크게 얘기해줘!");
		}
		else if(num == 17)
		{
			F_ScriptString = TEXT("미안, 내가 잘 듣지 못한 것 같아. 다른 궁금한 점은 없니?");
		}
		else if(num == 18)
		{
			F_ScriptString = TEXT("음, 다시 물어봐 줄 수 있을까?");
		}
	}
	
	SetSpeechBalloon();
}

void ALotteGameModeBase::State331()
{
	int32 num = FMath::RandRange(19,25);

	FString front = "S00";
	FString number = front.Append(FString::FromInt(num));
	FString full = number.Append(".wav");

	F_FileString = full;
	
	if(num == 19)
	{
		F_ScriptString = TEXT("오늘 롯데월드에서 몇 시까지 재미나게 놀 수 있는지 궁금하진 않니?");
	}
	else if(num == 20)
	{
		F_ScriptString = TEXT("오늘 퍼레이드 시간이 궁금하진 않니?");
	}
	else if(num == 21)
	{
		F_ScriptString = TEXT("나랑 같이 사진 찍는 건 어때? 로티와 함께 즐거운 추억을 남겨보자구!");
	}
	else if(num == 22)
	{
		F_ScriptString = TEXT("궁금한 놀이기구가 있다면 내가 알려줄게!");
	}
	else if(num == 23)
	{
		F_ScriptString = TEXT("궁금한 어트랙션은 없니?");
	}
	else if(num == 24)
	{
		F_ScriptString = TEXT("오늘 공연 시간을 알려줄 수도 있어!");
	}
	else if(num == 25)
	{
		F_ScriptString = TEXT("로티가 놀이기구 추천해줄까?");
	}
	
	SetSpeechBalloon();
}

void ALotteGameModeBase::State340()
{
	SetSpeechBalloon();
}

void ALotteGameModeBase::State360()
{
	if(F_EventString == TEXT("기념일"))
	{
		FTimerHandle timer;

		GetWorldTimerManager().SetTimer(timer, this, &ALotteGameModeBase::TalkStartCheck, 2, false);
		
		StartCake();
	}
	else
	{
		FTimerHandle timer;

		GetWorldTimerManager().SetTimer(timer, this, &ALotteGameModeBase::TalkStartCheck, 2, false);
		
		StartHurray();
	}
}

void ALotteGameModeBase::State400()
{
	StartBoring();
	
	GetTime();
}

void ALotteGameModeBase::State600_1()
{
	F_StateString = "600_1";
	F_ScriptString = TEXT("우리 같이 사진 찍자! 여기 내 옆으로 와봐. 나도 포즈를 취해볼게.");
	F_FileString = "Photo_600_1.wav";
	F_GestureString = "positive";

	SetSpeechBalloon();
}

void ALotteGameModeBase::State600_Fireworks()
{
	F_FileString = "Photo_Fireworks.wav";
	F_StateString = "600_Fire";
	
	TalkStartCheck();
}

void ALotteGameModeBase::State600_2()
{
	F_StateString = "600_2";
	F_ScriptString = TEXT("이번엔 롯데월드 포토존으로 가보자! 따라와!");
	F_FileString = "Photo_600_2.wav";
	F_GestureString = "positive";

	SetSpeechBalloon();
}

void ALotteGameModeBase::State600_3()
{
	F_StateString = "600_3";
	F_ScriptString = TEXT("이번엔 로티가 롯데월드 밖으로 안내해 줄게! 재밌는 모험을 떠나보자!");
	F_FileString = "Photo_600_3.wav";
	F_GestureString = "positive";

	SetSpeechBalloon();
}

void ALotteGameModeBase::State600_Result()
{
	F_StateString = "600_4";
	F_ScriptString = TEXT("사진이 마음에 들면 좋겠어! 다음에 또 만나자!");
	F_FileString = "Photo_Result.wav";
	F_GestureString = "positive";

	SetSpeechBalloon();
}

void ALotteGameModeBase::State702(int32 idx)
{
	FString front = "Question_";
	FString number = front.Append(FString::FromInt(idx+1));
	FString full = number.Append(".wav");
	
	F_FileString = full;

	if(idx == 0)
	{
		F_ScriptString = TEXT("너에 대해서 알려주면, 네가 좋아할 만한 놀이기구를 추천해 줄게! 이제부터 내가 하는 질문에 대해 앞에 있는 스크린을 터치해서 알려줘! 겨울의 차가운 공기는 너를 기분 좋게 만들어?");
	}
	else if(idx == 1)
	{
		F_ScriptString = TEXT("혼자서 무서운 영화 볼 수 있어?");
	}
	else if(idx == 2)
	{
		F_ScriptString = TEXT("번지점프를 한다는 상상을 해봐! 재미있을 것 같아?");
	}
	else if(idx == 3)
	{
		F_ScriptString = TEXT("어린이가 탈 수 있는 놀이기구 중에 추천해 줄까?");
	}

	SetSpeechBalloon();
}

void ALotteGameModeBase::State706(FString result)
{
	FString front = "Result_";
	FString number = front.Append(result);
	FString full = number.Append(".wav");
	
	F_FileString = full;

	if(result == "ITC")
	{
		Branch_1F_Attraction_Asset(TEXT("신밧드의모험"), TEXT("소개"));
	}
	else if(result == "ITA")
	{
		Branch_2F_Attraction_Asset(TEXT("후렌치레볼루션"), TEXT("소개"));
	}
	else if(result == "ISC")
	{
		Branch_1F_Attraction_Asset(TEXT("회전목마"), TEXT("소개"));
	}
	else if(result == "ISA")
	{
		Branch_4F_Attraction_Asset(TEXT("풍선비행"), TEXT("소개"));
	}
	else if(result == "OTC")
	{
		Branch_Magic_Attraction_Asset(TEXT("쁘띠빵빵"), TEXT("소개"));
	}
	else if(result == "OTA")
	{
		Branch_Magic_Attraction_Asset(TEXT("아트란티스"), TEXT("소개"));
	}
	else if(result == "OSC")
	{
		Branch_Magic_Attraction_Asset(TEXT("머킹의 회전목마"), TEXT("소개"));
	}
	else if(result == "OSA")
	{
		Branch_Magic_Attraction_Asset(TEXT("회전그네"), TEXT("소개"));
	}
}

void ALotteGameModeBase::StartBalloonSpeak()
{
	TalkStartCheck();	
}

void ALotteGameModeBase::State350(FString Sub_Category, FString Location, FString Entity,
                                  FString Asset)
{
	if(Location == TEXT("어드벤처 1층"))
	{
		if(Sub_Category == TEXT("어트랙션"))
		{
			Branch_1F_Attraction_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("식당가"))
		{
			Branch_1F_Dining_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("기프트샵"))
		{
			Branch_1F_Gift_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("체험플레이샵"))
		{
			Branch_1F_Play_Asset(Entity, Asset);
		}
		if(Sub_Category == TEXT("민속박물관저자거리"))
		{
			Branch_Museum_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("편의시설"))
		{
			Branch_1F_Amenities_Asset(Entity);
		}
	}
	else if(Location == TEXT("어드벤처 2층"))
	{
		if(Sub_Category == TEXT("어트랙션"))
		{
			Branch_2F_Attraction_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("식당가"))
		{
			Branch_2F_Dining_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("기프트샵"))
		{
			Branch_2F_Gift_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("체험플레이샵"))
		{
			Branch_2F_Play_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("편의시설"))
		{
			Branch_2F_Amenities_Asset(Entity);
		}
	}
	else if(Location == TEXT("어드벤처 3층"))
	{
		if(Sub_Category == TEXT("어트랙션"))
		{
			Branch_3F_Attraction_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("식당가"))
		{
			Branch_3F_Dining_Asset(Entity, Asset);
		}
		if(Sub_Category == TEXT("민속박물관저자거리"))
		{
			Branch_Museum_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("편의시설"))
		{
			Branch_3F_Amenities_Asset(Entity);
		}
	}
	else if(Location == TEXT("어드벤처 4층"))
	{
		if(Sub_Category == TEXT("어트랙션"))
		{
			Branch_4F_Attraction_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("식당가"))
		{
			Branch_4F_Dining_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("기프트샵"))
		{
			Branch_4F_Gift_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("체험플레이샵"))
		{
			Branch_4F_Play_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("편의시설"))
		{
			Branch_4F_Amenities_Asset(Entity);
		}
		if(Sub_Category == TEXT("사진관"))
		{
			Branch_4F_Attraction_Asset(Entity, Asset);
		}
	}
	else if(Location == TEXT("매직아일랜드"))
	{
		if(Sub_Category == TEXT("어트랙션"))
		{
			Branch_Magic_Attraction_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("식당가"))
		{
			Branch_Magic_Dining_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("기프트샵"))
		{
			Branch_Magic_Gift_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("편의시설"))
		{
			Branch_Magic_Amenities_Asset(Entity);
		}
	}
	else if(Location == TEXT("언더랜드"))
	{
		if(Sub_Category == TEXT("어트랙션"))
		{
			Branch_Under_Attraction_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("식당가"))
		{
			Branch_Under_Dining_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("기프트샵"))
		{
			Branch_Under_Gift_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("체험플레이샵"))
		{
			Branch_Under_Play_Asset(Entity, Asset);
		}
		else if(Sub_Category == TEXT("편의시설"))
		{
			Branch_Under_Amenities_Asset(Entity);
		}
	}
}

void ALotteGameModeBase::StartAsset()
{
	PlayInfoMontage();

	Set0001();

	FTimerHandle time;
	GetWorldTimerManager().SetTimer(time, this, &ALotteGameModeBase::TalkStartCheck, 1.0f, false);
}

void ALotteGameModeBase::Set0001()
{
	int idx = FMath::RandRange(1, 3);

	if(idx == 1)
	{
		F_0001String = "0001.wav";
	}
	else if(idx == 2)
	{
		F_0001String = "0002.wav";
	}
	else if(idx == 3)
	{
		F_0001String = "0003.wav";
	}
	
	SetTrack0001();
}

void ALotteGameModeBase::SetSpeechBalloon()
{
	FString string1 = F_ScriptString.Replace(TEXT("\\n"), TEXT(""));
	F_ScriptString = string1;
	
	FString string2 = F_ScriptString.Replace(TEXT("\\"), TEXT(""));
	F_ScriptString = string2;
	
	int32 idx = F_ScriptString.Len();
	
	UE_LOG(LogTemp, Warning, TEXT("string len : %d"), idx);
	
	if(idx < 50)
	{
		SetBalloon1();

		BalloonIdx = 1;
	}
	else if(idx < 400)
	{
		SetBalloon2();

		BalloonIdx = 2;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("string len is too long : %d"), idx);
	}
}

void ALotteGameModeBase::Branch_1F_Attraction_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("신밧드의모험"))
	{
		FString Name = "BP_Adventures_Of_Sinbad";

		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("스페인해적선"))
	{
		FString Name = "BP_The_Spain";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("후룸라이드"))
	{
		FString Name = "BP_Flume_Ride";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("회전바구니"))
	{
		FString Name = "BP_Drunken_Basket";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("게임팩토리"))
	{
		FString Name = "BP_Game_Factory";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("툼 오브 호러(유료)"))
	{
		FString Name = "BP_Tomb_of_Horror";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("로티트레인"))
	{
		FString Name = "BP_Lotty_Train";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("배틀그라운드 월드 에이전트"))
	{
		FString Name = "BP_Battlegrounds";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("플라이벤처"))
	{
		FString Name = "BP_Fly_Venture";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("회전목마"))
	{
		FString Name = "BP_Carousel";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("3D 황야의 무법자 Ⅱ"))
	{
		FString Name = "BP_Desperados";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("키즈토리아"))
	{
		FString Name = "BP_Kidstoria";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("드래곤 와일드슈팅"))
	{
		FString Name = "BP_Dragon_Wild_Shooting";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("어린이범퍼카"))
	{
		FString Name = "BP_Kids_Bumper_Car";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("햇님달님"))
	{
		FString Name = "BP_Moon_And_Suns";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("점핑피쉬"))
	{
		FString Name = "BP_Jumping_Fish";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("언더씨킹덤"))
	{
		FString Name = "BP_Undersea_Kingdom";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("유레카"))
	{
		FString Name = "BP_Eureka";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("스윙팡팡"))
	{
		FString Name = "BP_Swing_Pang_Pang";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("매직붕붕카"))
	{
		FString Name = "BP_Magic_Bong_Bong_Car";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("어린이 동화극장"))
	{
		FString Name = "BP_Fairytale_Theater";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("카트라이더레이싱월드"))
	{
		FString Name = "BP_Kartrider_Racing";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
}

void ALotteGameModeBase::Branch_1F_Dining_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("크리스탈팰리스(T.G.I.F)"))
	{
		FString Name = "BP_Crystal_Palace";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("스쿨푸드"))
	{
		FString Name = "BP_SchoolFood";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("플라이브이스낵"))
	{
		FString Name = "BP_Fly_V_Snack";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("스위티박스"))
	{
		FString Name = "BP_Sweetbox";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("BHC 치킨"))
	{
		FString Name = "BP_BHC";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("메리고라운드"))
	{
		FString Name = "BP_MerryGoRound";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("아라비안바자"))
	{
		FString Name = "BP_Arabian_Bazaar";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("스낵산호세"))
	{
		FString Name = "BP_Snack_Sanhose";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("오마스딜라이트"))
	{
		FString Name = "BP_OmarsDelight";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("칠링스테이션"))
	{
		FString Name = "BP_ChillingStaion";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("팝콘팩토리"))
	{
		FString Name = "BP_PopcornFactory";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("비트박스"))
	{
		FString Name = "BP_Beatbox";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("트리블하우스(설빙)"))
	{
		FString Name = "BP_TreebleHouse";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("폴바셋"))
	{
		FString Name = "BP_PaulBassett";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("루이스카우타운(파스퇴르밀크바)"))
	{
		FString Name = "BP_Pasteur_Milkbar";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("카페 아리아나(투썸플레이스)"))
	{
		FString Name = "BP_TwosomePlace";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("로티로리가든(더 쓰리위시스)"))
	{
		FString Name = "BP_LottyLorryGarden";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("아쿠아프라자(버거베어)"))
	{
		FString Name = "BP_Aquaplaza";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("카페다쥬르"))
	{
		FString Name = "BP_CafeDazur";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
}

void ALotteGameModeBase::Branch_1F_Gift_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("벨리곰 기프트샵"))
	{
		FString Name = "BP_BellyGom_Gift";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
	else if(Entity == TEXT("로티스 엠포리움"))
	{
		FString Name = "BP_LottisEmporium";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
	else if(Entity == TEXT("토이플러스"))
	{
		FString Name = "BP_ToyPlus";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
	else if(Entity == TEXT("사진상점"))
	{
		FString Name = "BP_Picture_Shop";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
	else if(Entity == TEXT("스위트월드(위니비니)"))
	{
		FString Name = "BP_SweetWorld";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
}

void ALotteGameModeBase::Branch_1F_Play_Asset(FString Entity, FString Asset)
{
	// 셋다 기프트에 포함이라 함수 하나로 통일
	if(Entity == TEXT("벨리곰 미스터리 맨션"))
	{
		FString Name = "BP_BellyGom_mansion";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
	else if(Entity == TEXT("도토리 캐리커쳐"))
	{
		FString Name = "BP_Dotori";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
	else if(Entity == TEXT("샤론캣 드레스"))
	{
		FString Name = "BP_SharonCat_dress";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
}

void ALotteGameModeBase::Branch_Museum_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("민속 박물관"))
	{
		FString Name = "BP_1F_Museum";
		
		if(Asset == TEXT("위치"))
		{
			Map_1F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
	}
	else if(Entity == TEXT("저자거리"))
	{
		FString Name = "BP_Street";
		
		if(Asset == TEXT("위치"))
		{
			Map_3F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
	}
}

void ALotteGameModeBase::Branch_1F_Amenities_Asset(FString Entity)
{
	if(Entity == TEXT("현금지급기"))
	{
		FString Name = "BP_1F_ATM";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("보조배터리 대여장소"))
	{
		FString Name = "BP_1F_Battery";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("유아 휴게실"))
	{
		FString Name = "BP_1F_Children_Lounge";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("손님 상담실"))
	{
		FString Name = "BP_1F_Customer";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("음수대"))
	{
		FString Name = "BP_1F_Drinking";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("엘레베이터"))
	{
		FString Name = "BP_1F_Elevator";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("에스컬레이터"))
	{
		FString Name = "BP_1F_Escalator";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("무인 키오스크"))
	{
		FString Name = "BP_1F_Kiosk";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("물품보관함"))
	{
		FString Name = "BP_1F_Locker";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("분실물 보관소"))
	{
		FString Name = "BP_1F_Lost_Found";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("의무실"))
	{
		FString Name = "BP_1F_Medical_Office";
		
		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("만남의 장소"))
	{
		FString Name = "BP_1F_Meeting";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("미아보호소"))
	{
		FString Name = "BP_1F_Missing_Children";
		
		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("포토스팟"))
	{
		FString Name = "BP_1F_PhotoSpot";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("계단"))
	{
		FString Name = "BP_1F_Stairs";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("유모차 대여소"))
	{
		FString Name = "BP_1F_Stroller_Rental_Office";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("화장실"))
	{
		FString Name = "BP_1F_Toilet";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("휠체어 대여소"))
	{
		FString Name = "BP_1F_Wheelchair";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("여성용품"))
	{
		FString Name = "BP_1F_Women_Goods";

		Map_1F_Amenities(Name);
	}
	else if(Entity == TEXT("피크닉"))
	{
		FString Name = "BP_1F_Picnic";

		Map_1F_Amenities(Name);
	}
}

void ALotteGameModeBase::Branch_2F_Attraction_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("후렌치레볼루션"))
	{
		FString Name = "BP_French_Revolution";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("범퍼카(어드벤처)"))
	{
		FString Name = "BP_Bumper_Car";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
}

void ALotteGameModeBase::Branch_2F_Dining_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("라쿠치나(김피라)"))
	{
		FString Name = "BP_Lacucina";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("델키스"))
	{
		FString Name = "BP_Delkys";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("라인랜드 퀴즈노스"))
	{
		FString Name = "BP_Line_Land";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("라팔로마(라라코스트)"))
	{
		FString Name = "BP_Lapaloma";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("상하이 꽁시면관"))
	{
		FString Name = "BP_Shanghai";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("앨리스하우스(오이씨푸드)"))
	{
		FString Name = "BP_Oishi_Food";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("앤티앤스 프레즐"))
	{
		FString Name = "BP_Shedeser_AntiAnes";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("델리본"))
	{
		FString Name = "BP_Delibon";
		
		if(Asset == TEXT("위치"))
		{
			Map_2F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("공차"))
	{
		FString Name = "BP_2F_Gongcha";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("잠바주스"))
	{
		FString Name = "BP_Jamba_Juice";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("아이스팩토리"))
	{
		FString Name = "BP_Ice_Factory";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
}

void ALotteGameModeBase::Branch_2F_Gift_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("인생네컷"))
	{
		FString Name = "BP_Bonita_Picture";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
	else if(Entity == TEXT("168CM"))
	{
		FString Name = "BP_168CM";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
	else if(Entity == TEXT("스위트존(위니비니)"))
	{
		FString Name = "BP_Sweet_Zone";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
}

void ALotteGameModeBase::Branch_2F_Play_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("매직스쿨 분장실"))
	{
		FString Name = "BP_MagicSchool";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("아벤투라(타로카드)"))
	{
		FString Name = "BP_Aventura";

		if(Asset == TEXT("위치"))
		{
			Map_2F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Gift(Name);
		}
	}
}

void ALotteGameModeBase::Branch_2F_Amenities_Asset(FString Entity)
{
	if(Entity == TEXT("보조배터리 대여장소"))
	{
		FString Name = "BP_2F_Battery";

		Map_2F_Amenities(Name);
	}
	else if(Entity == TEXT("음수대"))
	{
		FString Name = "BP_2F_Drinking";

		Map_2F_Amenities(Name);
	}
	else if(Entity == TEXT("엘레베이터"))
	{
		FString Name = "BP_2F_Elevator";

		Map_2F_Amenities(Name);
	}
	else if(Entity == TEXT("에스컬레이터"))
	{
		FString Name = "BP_2F_Escalator";

		Map_2F_Amenities(Name);
	}
	else if(Entity == TEXT("물품보관함"))
	{
		FString Name = "BP_2F_Locker";

		Map_2F_Amenities(Name);
	}
	else if(Entity == TEXT("계단"))
	{
		FString Name = "BP_2F_Stairs";

		Map_2F_Amenities(Name);
	}
	else if(Entity == TEXT("화장실"))
	{
		FString Name = "BP_2F_Toilet";

		Map_2F_Amenities(Name);
	}
}

void ALotteGameModeBase::Branch_3F_Attraction_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("게임빌리지"))
	{
		FString Name = "BP_Game_Village";
		
		if(Asset == TEXT("위치"))
		{
			Map_3F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("월드모노레일"))
	{
		FString Name = "BP_World_Monorail";

		if(Asset == TEXT("위치"))
		{
			Map_3F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
}

void ALotteGameModeBase::Branch_3F_Dining_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("정글스낵"))
	{
		FString Name = "BP_Jungle_Snack";

		if(Asset == TEXT("위치"))
		{
			Map_3F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("엠테이블"))
	{
		FString Name = "BP_Mtable";

		if(Asset == TEXT("위치"))
		{
			Map_3F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
}

void ALotteGameModeBase::Branch_3F_Amenities_Asset(FString Entity)
{
	if(Entity == TEXT("음수대"))
	{
		FString Name = "BP_3F_Drinking";

		Map_3F_Amenities(Name);
	}
	else if(Entity == TEXT("엘레베이터"))
	{
		FString Name = "BP_3F_Elevator";

		Map_3F_Amenities(Name);
	}
	else if(Entity == TEXT("에스컬레이터"))
	{
		FString Name = "BP_3F_Escalator";

		Map_3F_Amenities(Name);
	}
	else if(Entity == TEXT("계단"))
	{
		FString Name = "BP_3F_Stairs";

		Map_3F_Amenities(Name);
	}
	else if(Entity == TEXT("화장실"))
	{
		FString Name = "BP_3F_Toilet";

		Map_3F_Amenities(Name);
	}
	else if(Entity == TEXT("무슬림 기도실"))
	{
		FString Name = "BP_3F_Muslim";

		Map_3F_Amenities(Name);
	}
}

void ALotteGameModeBase::Branch_4F_Attraction_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("풍선비행"))
	{
		FString Name = "BP_Balloon_Ride";

		if(Asset == TEXT("위치"))
		{
			Map_4F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("슬릭스튜디오"))
	{
		FString Name = "BP_Slick_Studio";

		if(Asset == TEXT("위치"))
		{
			Map_4F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("전망대 게임장 펀시티"))
	{
		FString Name = "BP_GameZone";

		if(Asset == TEXT("위치"))
		{
			Map_4F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("어크로스다크"))
	{
		FString Name = "BP_Across_Dark";

		if(Asset == TEXT("위치"))
		{
			Map_4F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("파라오의 분노"))
	{
		FString Name = "BP_Pharaohs_Fury";

		if(Asset == TEXT("위치"))
		{
			Map_4F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
}

void ALotteGameModeBase::Branch_4F_Dining_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("스핑크스스낵(뉴욕핫도그)"))
	{
		FString Name = "BP_Newyork_Hotdog";

		if(Asset == TEXT("위치"))
		{
			Map_4F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("파라오스낵"))
	{
		FString Name = "BP_Pharaoh_Snack";
		
		if(Asset == TEXT("위치"))
		{
			Map_4F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("오벨리스크"))
	{
		FString Name = "BP_Obelisk";

		if(Asset == TEXT("위치"))
		{
			Map_4F_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
}

void ALotteGameModeBase::Branch_4F_Gift_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("GIFT SHOP(헬로키티)"))
	{
		FString Name = "BP_Giftshop";

		if(Asset == TEXT("위치"))
		{
			Map_4F_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
}

void ALotteGameModeBase::Branch_4F_Play_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("루프탑 갤러리"))
	{
		FString Name = "BP_Rooftop_Gallery";

		if(Asset == TEXT("위치"))
		{
			Map_4F_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
}

void ALotteGameModeBase::Branch_4F_Amenities_Asset(FString Entity)
{
	if(Entity == TEXT("엘레베이터"))
	{
		FString Name = "BP_4F_Elevator";

		Map_4F_Amenities(Name);
	}
	else if(Entity == TEXT("에스컬레이터"))
	{
		FString Name = "BP_4F_Escalator";

		Map_4F_Amenities(Name);
	}
	else if(Entity == TEXT("계단"))
	{
		FString Name = "BP_4F_Stairs";

		Map_4F_Amenities(Name);
	}
	else if(Entity == TEXT("화장실"))
	{
		FString Name = "BP_4F_Toilet";

		Map_4F_Amenities(Name);
	}
}

void ALotteGameModeBase::Branch_Magic_Attraction_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("5G 아트란티스"))
	{
		FString Name = "BP_5G_Atlantis";
		
		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("자이로드롭"))
	{
		FString Name = "BP_Gyro_Drop";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("자이로스윙"))
	{
		FString Name = "BP_Gyro_Swing";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("자이로스핀"))
	{
		FString Name = "BP_Gyro_Spin";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("아트란티스"))
	{
		FString Name = "BP_Atlantis";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("혜성특급"))
	{
		FString Name = "BP_Comet_Express";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("번지드롭"))
	{
		FString Name = "BP_Bungee_Drop";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("회전그네"))
	{
		FString Name = "BP_Swing_Tree";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("머킹의 회전목마"))
	{
		FString Name = "BP_Merking_royal_pageant";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("문보트(유료)"))
	{
		FString Name = "BP_MoonBoat";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("쁘띠빵빵"))
	{
		FString Name = "BP_Petit_Pang_Pang";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("환타지드림"))
	{
		FString Name = "BP_Fantasy_Dreams";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("바덴바덴 카니발"))
	{
		FString Name = "BP_Baden_Baden_Carnival";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
}

void ALotteGameModeBase::Branch_Magic_Dining_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("레이크뷰(T.G.I.F)"))
	{
		FString Name = "BP_LakeView";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("비비큐빌리지"))
	{
		FString Name = "BP_BBQ_Village";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("레이크푸드(스쿨스토어)"))
	{
		FString Name = "BP_LakeFood";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("후르츠월드"))
	{
		FString Name = "BP_FruitWorld";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("캔디캐슬"))
	{
		FString Name = "BP_CandyCastle";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("칠링아일랜드"))
	{
		FString Name = "BP_ChillingIsland";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("아일랜드 기프트샵(짤즈빵 젤라또)"))
	{
		FString Name = "BP_IslandGiftShop";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("매직트럭(밀키)"))
	{
		FString Name = "BP_MagicTruck";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("꼰 푸루따"))
	{
		FString Name = "BP_Cconcooruta";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("서호정(투썸플레이스)"))
	{
		FString Name = "BP_Seohojeong";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("공차"))
	{
		FString Name = "BP_Magic_Gongcha";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
}

void ALotteGameModeBase::Branch_Magic_Gift_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("매직캐슬 트레져스"))
	{
		FString Name = "BP_MagicCastleTreasures";

		if(Asset == TEXT("위치"))
		{
			Map_Magic_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
}

void ALotteGameModeBase::Branch_Magic_Amenities_Asset(FString Entity)
{
	if(Entity == TEXT("현금지급기"))
	{
		FString Name = "BP_Magic_ATM";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("보조배터리 대여장소"))
	{
		FString Name = "BP_Magic_Battery";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("유아 휴게실"))
	{
		FString Name = "BP_Magic_Children_Lounge";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("음수대"))
	{
		FString Name = "BP_Magic_Drinking";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("엘레베이터"))
	{
		FString Name = "BP_Magic_Elevator";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("에스컬레이터"))
	{
		FString Name = "BP_Magic_Escalator";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("무인 키오스크"))
	{
		FString Name = "BP_Magic_Kiosk";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("물품보관함"))
	{
		FString Name = "BP_Magic_Locker";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("의무실"))
	{
		FString Name = "BP_Magic_Medical_Office";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("포토스팟"))
	{
		FString Name = "BP_Magic_PhotoSpot";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("계단"))
	{
		FString Name = "BP_Magic_Stairs";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("화장실"))
	{
		FString Name = "BP_Magic_Toilet";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("여성용품"))
	{
		FString Name = "BP_Magic_Women_Goods";

		Map_Magic_Amenities(Name);
	}
	else if(Entity == TEXT("흡연실"))
	{
		FString Name = "BP_Magic_Smoking_Room";

		Map_Magic_Amenities(Name);
	}
}

void ALotteGameModeBase::Branch_Under_Attraction_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("드림보트"))
	{
		FString Name = "BP_DreamBoats";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("와일드 밸리"))
	{
		FString Name = "BP_WildValley";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("와일드 정글"))
	{
		FString Name = "BP_WildJungle";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("와일드 윙"))
	{
		FString Name = "BP_WildWing";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
	else if(Entity == TEXT("4D슈팅시어터"))
	{
		FString Name = "BP_4DShootingTheater";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Attraction(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Attraction(Name, 0);
		}
		else if(Asset == TEXT("운영"))
		{
			Info_Attraction(Name, 1);
		}
		else if(Asset == TEXT("운휴"))
		{
			Info_Attraction(Name, 2);
		}
	}
}

void ALotteGameModeBase::Branch_Under_Dining_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("101번지 남산돈까스"))
	{
		FString Name = "BP_101Namsan_Tonkastu";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("페어리테일(도미노피자)"))
	{
		FString Name = "BP_FairyTale";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("유브유부"))
	{
		FString Name = "BP_Youveyoubu";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("츄로빅"))
	{
		FString Name = "BP_Churrovic";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("오거스 팩토리(롯데리아 언더랜드점)"))
	{
		FString Name = "BP_OgresFactory";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Dining(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
}

void ALotteGameModeBase::Branch_Under_Gift_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("환타지 기프트샵"))
	{
		FString Name = "BP_FantasyGifts";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
	else if(Entity == TEXT("픽시매직"))
	{
		FString Name = "BP_Pixiemagic";
		
		if(Asset == TEXT("위치"))
		{
			Map_Under_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
}

void ALotteGameModeBase::Branch_Under_Play_Asset(FString Entity, FString Asset)
{
	if(Entity == TEXT("샤론캣 프리미엄라운지"))
	{
		FString Name = "BP_Sharoncat_Premium";

		if(Asset == TEXT("위치"))
		{
			Map_Under_Gift(Name);
		}
		else if(Asset == TEXT("소개"))
		{
			Info_Dining(Name);
		}
	}
}

void ALotteGameModeBase::Branch_Under_Amenities_Asset(FString Entity)
{
	if(Entity == TEXT("보조배터리 대여장소"))
	{
		FString Name = "BP_Underland_Battery";

		Map_Under_Amenities(Name);
	}
	else if(Entity == TEXT("엘레베이터"))
	{
		FString Name = "BP_Underland_Elevator";

		Map_Under_Amenities(Name);
	}
	else if(Entity == TEXT("에스컬레이터"))
	{
		FString Name = "BP_Underland_Escalator";

		Map_Under_Amenities(Name);
	}
	else if(Entity == TEXT("물품보관함"))
	{
		FString Name = "BP_Underland_Locker";

		Map_Under_Amenities(Name);
	}
	else if(Entity == TEXT("계단"))
	{
		FString Name = "BP_Underland_Stairs";

		Map_Under_Amenities(Name);
	}
	else if(Entity == TEXT("화장실"))
	{
		FString Name = "BP_Underland_Toilet";

		Map_Under_Amenities(Name);
	}
}

void ALotteGameModeBase::LotteTime351(FString sub_category)
{
	if(F_TimeString.Len() == 13)
	{
		FString date = F_TimeString.Mid(0, 4);
		FString day = F_TimeString.Mid(4, 1);
		FString startTime = F_TimeString.Mid(5, 4);
		FString EndTime = F_TimeString.Mid(9, 4);

		LotteTimeOn(sub_category, date, day, startTime, EndTime);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("롯데월드 오픈 시간 자릿수 틀림!!!!!!!"));
	}
}

void ALotteGameModeBase::TimeCheckFunc()
{
	int32 time = F_TimeString.Len()/4;
				
	UE_LOG(LogTemp, Warning, TEXT("공연 횟수 체크 : %d"), time);

	for(int i = 0; i < time; i++)
	{
		times.Add(F_TimeString.Mid(i*4, 4));
	}
}

void ALotteGameModeBase::PerformAsset351(FString entity, FString asset, FString location)
{
	times.Empty();
	
	// if(entity == TEXT("35주년 맵핑쇼"))
	// {
	// 	FString Name = "BP_MappingShow35";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("35주년 퍼레이드"))
	// {
	// 	FString Name = "BP_ParadeShow35";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("로티 프렌즈의 보물섬"))
	// {
	// 	FString Name = "BP_LottiFriendsTreasureLand";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("로티의 크리스마스 환타지"))
	// {
	// 	FString Name = "BP_ChristmasFantasy";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("마법성냥과 꿈꾸는 밤"))
	// {
	// 	FString Name = "BP_MagicMatchDreamNight";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("매직 인 더 나이트"))
	// {
	// 	FString Name = "BP_MagicInTheNight";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("모던 재즈 앙상블"))
	// {
	// 	FString Name = "BP_ModernJazzEnsemble";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("산타 빌리지 투어"))
	// {
	// 	FString Name = "BP_SantaVillageTour";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("색소폰 앙상블"))
	// {
	// 	FString Name = "BP_SaxophoneEnsemble";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("모던 팝 앙상블"))
	// {
	// 	FString Name = "BP_PopEnsemble";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("캐롤 밴드 쇼"))
	// {
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		FString Name = "BP_CarolBandShow";
	//
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		if(location == TEXT("어드벤처 1층 가든스테이지"))
	// 		{
	// 			FString Name = "BP_CarolBandShow2";
	//
	// 			PerformMap(Name);
	// 		}
	// 		else
	// 		{
	// 			FString Name = "BP_CarolBandShow1";
	// 			
	// 			PerformMap(Name);
	// 		}
	// 	}
	// }
	// else if(entity == TEXT("크리스마스 빅밴드쇼"))
	// {
	// 	FString Name = "BP_ChristmasBigBandShow";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("키디 밴드"))
	// {
	// 	FString Name = "BP_KiddyBand";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("해피 크리스마스 퍼레이드"))
	// {
	// 	FString Name = "BP_HappyChristmasParade";
	// 	
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		PerformMap(Name);
	// 	}
	// }
	// else if(entity == TEXT("스누피 포토 타임"))
	// {
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		FString Name = "BP_SnoopyPhotoTime";
	// 		
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		if(location == TEXT("어드벤처 2층 바르셀로나 광장"))
	// 		{
	// 			FString Name = "BP_SnoopyPhotoTime2";
	//
	// 			PerformMap(Name);
	// 		}
	// 		else
	// 		{
	// 			FString Name = "BP_SnoopyPhotoTime1";
	// 			
	// 			PerformMap(Name);
	// 		}
	// 	}
	// }
	// else if(entity == TEXT("Smile, With Friends"))
	// {
	// 	if(asset == TEXT("일정"))
	// 	{
	// 		FString Name = "BP_SmileWithFriends";
	// 		
	// 		if(F_TimeString == TEXT("금일 휴연"))
	// 		{
	// 			times.Add(F_TimeString);
	// 		}
	// 		else
	// 		{
	// 			TimeCheckFunc();
	// 		}
	// 		
	// 		PerformInfo(times, Name);
	// 	}
	// 	// 장소
	// 	else
	// 	{
	// 		if(location == TEXT("어드벤처 1층 키디존"))
	// 		{
	// 			FString Name = "BP_SmileWithFriends2";
	//
	// 			PerformMap(Name);
	// 		}
	// 		else
	// 		{
	// 			FString Name = "BP_SmileWithFriends1";
	// 			
	// 			PerformMap(Name);
	// 		}
	// 	}
	// }

	//////////////////////////////////////////////////////////// 1월 공연
	if(entity == TEXT("로티스 어드벤처 퍼레이드"))
	{
		FString Name = "BP_RotisAdventureParade";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("민속 한마당"))
	{
		FString Name = "BP_MinsokFestival";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("35주년 퍼레이드"))
	{
		FString Name = "BP_ParadeShow35";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("마법성냥과 꿈꾸는 밤"))
	{
		FString Name = "BP_MagicMatchDreamNight";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("시그니처 밴드 쇼"))
	{
		FString Name = "BP_SignatureBandShow";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("월드 팝 밴드 쇼"))
	{
		FString Name = "BP_WorldPopBandShow";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("빅 밴드 쇼"))
	{
		FString Name = "BP_BigBandShow";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("모던 팝 앙상블"))
	{
		FString Name = "BP_PopEnsemble";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("키디 밴드"))
	{
		FString Name = "BP_KiddyBand";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("딕시 밴드"))
	{
		FString Name = "BP_DixieBand";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("로티 프렌즈의 보물섬"))
	{
		FString Name = "BP_LottiFriendsTreasureLand";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("로티프렌즈 싱어롱"))
	{
		FString Name = "BP_LottiFriendsSingASong";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("매직 캐슬 라이츠업"))
	{
		FString Name = "BP_MagicCastleLightUp";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("매직 인 더 나이트"))
	{
		FString Name = "BP_MagicInTheNight";
		
		if(asset == TEXT("일정"))
		{
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
			
			PerformInfo(times, Name);
		}
		// 장소
		else
		{
			PerformMap(Name);
		}
	}
	else if(entity == TEXT("Smile, With Friends"))
	{
		if(asset == TEXT("일정"))
		{
			FString Name = "BP_SmileWithFriends";
				
			if(F_TimeString == TEXT("금일 휴연"))
			{
				times.Add(F_TimeString);
			}
			else
			{
				TimeCheckFunc();
			}
				
			PerformInfo(times, Name);
		}
			// 장소
		else
		{
			if(location == TEXT("어드벤처 1층 키디존"))
			{
				FString Name = "BP_SmileWithFriends2";
		
				PerformMap(Name);
			}
			else
			{
				FString Name = "BP_SmileWithFriends1";
					
				PerformMap(Name);
			}
		}
	}
	else
	{
		CallBackFSM1();

		UE_LOG(LogTemp, Error, TEXT("perform name is : %s"), *entity);
	}
}

void ALotteGameModeBase::PerformListTime351(FString entity)
{
	PerformListStrings.Empty();
	
	for(const TSharedPtr<FJsonValue>& value : PerformList)
	{
		TSharedPtr<FJsonObject> ArrayObject = value->AsObject();

		if(ArrayObject.IsValid())
		{
			FString name = ArrayObject->GetStringField(TEXT("parade_name"));
			FString loc = ArrayObject->GetStringField(TEXT("parade_loc"));
			FString time = ArrayObject->GetStringField(TEXT("open_at"));

			UE_LOG(LogTemp, Warning, TEXT("name is : %s, loc is : %s, time is : %s"), *name, *loc, *time);
			
			PerformListStrings.Add(name);
			PerformListStrings.Add(loc);
			PerformListStrings.Add(time);

			UE_LOG(LogTemp, Warning, TEXT("perform num is %d"), PerformListStrings.Num());
		}
	}

	PerformListOn(entity, PerformListStrings);
}

void ALotteGameModeBase::TestAudio(const FString& string, const FString& audio, const FString& sub, const FString& location, const FString& entity, const FString& asset)
{
	F_StateString = "350";
	F_FileString = audio;
	F_EventString = TEXT("기념일");
	
	UE_LOG(LogTemp, Warning, TEXT("F_state : %s"), *F_StateString);
	UE_LOG(LogTemp, Warning, TEXT("F_file : %s"), *F_FileString);
	UE_LOG(LogTemp, Warning, TEXT("F_state : %s"), *sub);
	UE_LOG(LogTemp, Warning, TEXT("F_state : %s"), *location);
	UE_LOG(LogTemp, Warning, TEXT("F_state : %s"), *entity);
	UE_LOG(LogTemp, Warning, TEXT("F_state : %s"), *asset);
	
	//State350(sub, location, entity, asset);

	//State340();

	//F_TimeString = string;

	PerformAsset351(entity, asset, location);
}