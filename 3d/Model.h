#pragma once

#include "TextureManager.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <vector>
#include <wrl.h>

/// <summary>
/// 3Dモデル
/// </summary>
class Model {
public: // 列挙子
  /// <summary>
  /// ルートパラメータ番号
  /// </summary>
  enum class RoomParameter {
	kWorldTransform, // ワールド変換行列
	kViewProjection, // ビュープロジェクション変換行列
	kTexture,        // テクスチャ
  };

public: // サブクラス
  // 頂点データ構造体
  struct VertexPosNormalUv {
	DirectX::XMFLOAT3 pos;    // xyz座標
	DirectX::XMFLOAT3 normal; // 法線ベクトル
	DirectX::XMFLOAT2 uv;     // uv座標
  };

public: // 静的メンバ関数
  /// <summary>
  /// 静的初期化
  /// </summary>
  /// <param name="device">デバイス</param>
  /// <param name="window_width">画面幅</param>
  /// <param name="window_height">画面高さ</param>
  static void StaticInitialize(ID3D12Device* device, int window_width, int window_height);

  /// <summary>
  /// 描画前処理
  /// </summary>
  /// <param name="commandList">描画コマンドリスト</param>
  static void PreDraw(ID3D12GraphicsCommandList* commandList);

  /// <summary>
  /// 描画後処理
  /// </summary>
  static void PostDraw();

  /// <summary>
  /// 3Dモデル生成
  /// </summary>
  /// <returns></returns>
  static Model* Create();

private: // 静的メンバ変数
  // デバイス
  static ID3D12Device* sDevice;
  // デスクリプタサイズ
  static UINT sDescriptorHandleIncrementSize;
  // コマンドリスト
  static ID3D12GraphicsCommandList* sCommandList;
  // ルートシグネチャ
  static Microsoft::WRL::ComPtr<ID3D12RootSignature> sRootSignature;
  // パイプラインステートオブジェクト
  static Microsoft::WRL::ComPtr<ID3D12PipelineState> sPipelineState;

private: // 静的メンバ関数
  /// <summary>
  /// グラフィックパイプライン生成
  /// </summary>
  static void InitializeGraphicsPipeline();

public: // メンバ関数
  /// <summary>
  /// 初期化
  /// </summary>
  void Initialize();
  /// <summary>
  /// 描画
  /// </summary>
  void Draw(
    const WorldTransform& worldTransform, const ViewProjection& viewProjection,
    uint32_t textureHadle = 0);

  /// <summary>
  /// メッシュデータ生成
  /// </summary>
  void CreateMesh();

private: // メンバ変数
  // 頂点バッファビュー
  D3D12_VERTEX_BUFFER_VIEW vbView_;
  // インデックスバッファビュー
  D3D12_INDEX_BUFFER_VIEW ibView_;
  // 頂点データ配列
  std::vector<VertexPosNormalUv> vertices_;
  // 頂点インデックス配列
  std::vector<uint16_t> indices_;
  // 頂点バッファ
  Microsoft::WRL::ComPtr<ID3D12Resource> vertBuff_;
  // インデックスバッファ
  Microsoft::WRL::ComPtr<ID3D12Resource> indexBuff_;
};
