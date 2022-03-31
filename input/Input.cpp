#include "Input.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")

void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
  HRESULT result = S_FALSE;

  // DirectInputオブジェクトの生成
  result = DirectInput8Create(
    hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dInput_, nullptr);
  assert(SUCCEEDED(result));

  // キーボードデバイスの生成
  result = dInput_->CreateDevice(GUID_SysKeyboard, &devKeyboard_, NULL);
  assert(SUCCEEDED(result));

  // 入力データ形式のセット
  result = devKeyboard_->SetDataFormat(&c_dfDIKeyboard); // 標準形式
  assert(SUCCEEDED(result));

  // 排他制御レベルのセット
  result =
    devKeyboard_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
  assert(SUCCEEDED(result));
}

void Input::Update() {
  devKeyboard_->Acquire(); // キーボード動作開始

  // 前回のキー入力を保存
  keyPre_ = key_;

  // キーの入力
  devKeyboard_->GetDeviceState((DWORD)size(key_), key_.data());
}

bool Input::PushKey(BYTE keyNumber) {

  // 0でなければ押している
  if (key_[keyNumber]) {
	return true;
  }

  // 押していない
  return false;
}

bool Input::TriggerKey(BYTE keyNumber) {

  // 前回が0で、今回が0でなければトリガー
  if (!keyPre_[keyNumber] && key_[keyNumber]) {
	return true;
  }

  // トリガーでない
  return false;
}
