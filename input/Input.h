#pragma once

#include <array>
#include <Windows.h>
#include <wrl.h>

#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>

/// <summary>
/// 入力
/// </summary>
class Input {
public: // メンバ関数
  /// <summary>
  /// 初期化
  /// </summary>
  void Initialize(HINSTANCE hInstance, HWND hwnd);

  /// <summary>
  /// 毎フレーム処理
  /// </summary>
  void Update();

  /// <summary>
  /// キーの押下をチェック
  /// </summary>
  /// <param name="keyNumber">キー番号( DIK_0 等)</param>
  /// <returns>押されているか</returns>
  bool PushKey(BYTE keyNumber);

  /// <summary>
  /// キーのトリガーをチェック
  /// </summary>
  /// <param name="keyNumber">キー番号( DIK_0 等)</param>
  /// <returns>トリガーか</returns>
  bool TriggerKey(BYTE keyNumber);

  /// <summary>
  /// 全キー情報取得
  /// </summary>
  /// <param name="keyStateBuf">全キー情報</param>
  const std::array<BYTE, 256>& GetAllKey() { return key_; }

private: // メンバ変数
  Microsoft::WRL::ComPtr<IDirectInput8> dInput_;
  Microsoft::WRL::ComPtr<IDirectInputDevice8> devKeyboard_;
  std::array<BYTE, 256> key_;
  std::array<BYTE, 256> keyPre_;
};
