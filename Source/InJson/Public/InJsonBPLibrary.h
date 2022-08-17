#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "JsonUtilities.h"
#include "InJson.h"
#include "InJsonBPLibrary.generated.h"


UCLASS()
class UInJsonBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "InJson", CustomThunk, meta = (CustomStructureParam = "Struct", DisplayName = "StructToJsonString"))
	static bool StructToJsonString(FString& OutJsonString, const UStruct* Struct);

	UFUNCTION(BlueprintCallable, Category = "InJson", CustomThunk, meta = (CustomStructureParam = "StructToFill", DisplayName = "JsonStringToStruct"))
	static bool JsonStringToStruct(const FString& JsonString, UStruct*& StructToFill);


    DECLARE_FUNCTION(execStructToJsonString)
    {
        P_GET_PROPERTY_REF(FStrProperty, OutJsonString);

        Stack.StepCompiledIn<FStructProperty>(nullptr);
        void* InStruct = Stack.MostRecentPropertyAddress;

        P_FINISH;
        bool bSuccess = false;

        FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
        if (StructProp && InStruct)
        {
            UScriptStruct* StructType = StructProp->Struct;

            P_NATIVE_BEGIN;
            bSuccess = Inner_StructToJsonString(OutJsonString, StructType, InStruct);
            P_NATIVE_END;
        }
        *static_cast<bool*>(RESULT_PARAM) = bSuccess;
    }

    DECLARE_FUNCTION(execJsonStringToStruct)
    {
        P_GET_PROPERTY_REF(FStrProperty, JsonString);

        Stack.StepCompiledIn<FStructProperty>(NULL);
        void* StructPtr = Stack.MostRecentPropertyAddress;
        FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

        P_FINISH;

        UStruct* StructDefinition = StructProperty->Struct;
        bool bSuccess;

        P_NATIVE_BEGIN

        bSuccess = Inner_JsonStringToStruct(JsonString, StructDefinition, StructPtr);

        P_NATIVE_END

        *static_cast<bool*>(RESULT_PARAM) = bSuccess;
    }
private:
    static bool Inner_StructToJsonString(FString& OutJsonString, const UStruct* StructDefinition, const void* Struct)
    {
        FJsonObjectConverter::UStructToJsonObjectString(StructDefinition, Struct, OutJsonString, 0, 0);

        return !OutJsonString.IsEmpty();
    }
    static bool Inner_JsonStringToStruct(const FString& JsonString, UStruct* StructDefinition, void* StructPtr)
    {
        TSharedPtr<FJsonObject> JsonObject;
        const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
        if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
        {
            UE_LOG(LogInJsonModule, Warning, TEXT("JsonObjectStringToUStruct - Unable to parse json=[%s]"), *JsonString);
            return false;
        }

        if (!FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), StructDefinition, StructPtr, 0, 0))
        {
            UE_LOG(LogInJsonModule, Warning, TEXT("JsonObjectStringToUStruct - Unable to deserialize. json=[%s]"), *JsonString);
            return false;
        }

        return true;
    }
};
