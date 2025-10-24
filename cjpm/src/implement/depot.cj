// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.net.*
import std.io.*
import std.fs.*
import std.collection.*
import stdx.net.http.*
import stdx.net.tls.common.*
import stdx.net.tls.*
import stdx.compress.*
import cjpm.config.*

let tcpSocketConnector = {
    sa: SocketAddress =>
        let socket = TcpSocket(sa)
        socket.connect(timeout: Duration.second * 3)
        return socket
}

public class Depot {
    public var baseUrl: String = ""
    public var token: String = ""
    public var localRepo: String = ""

    public func loadConfiguration(): Bool {
        if (!FINAL_CJPM_CONFIG_INFO.repository.contains("home")) {
            eprintln("Error: get data of [repository.home] failed, please check if cjpm-config.toml is set up correctly")
            return false
        }

        this.baseUrl = FINAL_CJPM_CONFIG_INFO.repository["home"].registry
        this.token = FINAL_CJPM_CONFIG_INFO.repository["home"].token
        this.localRepo = Path(FINAL_CJPM_CONFIG_INFO.repository["local"].path).join("repository").toString()

        if (baseUrl.isEmpty()) {
            eprintln("Error: get registry of [repository.home] failed, please check if cjpm-config.toml is set up correctly")
            return false
        }

        return true
    }

    public static func new(): Option<Depot> {
        let instance: Depot = Depot()
        if (instance.loadConfiguration()) {
            return Option<Depot>.Some(instance)
        }
        return Option<Depot>.None
    }

    // publish data to central repository
    public func publish(tomlInfo: TomlInfo, data: Array<Byte>): (Bool, String) {
        let name = tomlInfo.pkg.name
        let version = tomlInfo.pkg.version
        let publishUrl = baseUrl + "/pkg/${name}"

        // building client
        var tlsConfig = TlsClientConfig()
        tlsConfig.verifyMode = TrustAll
        let client = ClientBuilder().connector(tcpSocketConnector).noProxy().tlsConfig(tlsConfig).build()

        // building https request
        var body = ByteBuffer()
        body.write(data)
        let request = HttpRequestBuilder().
            method("POST").
            url(publishUrl).
            header("Authorization", token).
            body(body).
            build()
        let resp = client.send(request)
        client.close()

        // verify response code
        match (resp.status) {
            case 200 => return (true, "${name}-${version} publish success")
            case 400 => return (false, "${name}-${version} publish failed with status ${resp.status}: invalid parameters")
            case 401 => return (false, "${name}-${version} publish failed with status ${resp.status}: authentication failed")
            case 403 => return (false, "${name}-${version} publish failed with status ${resp.status}: permission denied")
            case 404 => return (false, "${name}-${version} publish failed with status ${resp.status}: file does not exist")
            case s where (s >= 500) => return (false, "${name}-${version} publish failed with status ${resp.status}: server error")
            case _ => return (false, "${name}-${version} publish failed, unknown error with status ${resp.status}")
        }
    }

    // download package from central repository
    public func download(name: String, version: String): (Bool, String) {
        let downloadUrl = baseUrl + "/pkg/${name}/${version}"

        // building client
        var tlsConfig = TlsClientConfig()
        tlsConfig.verifyMode = TrustAll
        let client = ClientBuilder().connector(tcpSocketConnector).noProxy().tlsConfig(tlsConfig).build()

        // building https request
        let request = HttpRequestBuilder().method("GET").url(downloadUrl).build()
        let resp = client.send(request)
        client.close()

        // verify response code
        match (resp.status) {
            case 200 => ()
            case 400 => return (false, "${name}-${version} load failed with status ${resp.status}: invalid parameters")
            case 401 => return (false, "${name}-${version} load failed with status ${resp.status}: authentication failed")
            case 403 => return (false, "${name}-${version} load failed with status ${resp.status}: permission denied")
            case 404 => return (false, "${name}-${version} load failed with status ${resp.status}: file does not exist")
            case s where (s >= 500) => return (false, "${name}-${version} load failed with status ${resp.status}: server error")
            case _ => return (false, "${name}-${version} load failed, unknown error with status ${resp.status}")
        }

        // get response body
        // match (resp.body) {
        //     case s: Seekable =>
        //         if (s.length <= 0) {
        //             return (false, "${name}-${version} load failed with empty response data, 111")
        //         }
        //     case _ => return (false, "${name}-${version} load failed with empty response data, 222")
        // }

        let buf = Array<UInt8>(1024, repeat: 0)
        let buffer = ArrayList<UInt8>()
        var i = 0
        do {
            i = resp.body.read(buf)
            buffer.add(all: buf[..i])
        } while (i != 0)

        // save, uncompress and delete cjp file
        let repositoryPath = Path(this.localRepo).join("source").join("default").toString()
        let cjpTempPath = Path(repositoryPath).join("${name}-${version}.cjp")
        try {
            // save
            File.writeTo(cjpTempPath, buffer.toArray())

            // uncompress
            TarGzip.extract(fromTarGzip: cjpTempPath.toString(), destDir: repositoryPath, overwrite: true)
            return (true, "${name}-${version} load success")
        } catch (e: Exception) {
            eprintln(e.message)
            return (false, "${name}-${version} load failed with error while uncompressing and saving local repository")
        } finally {
            // delete cjp
            deleteFile(cjpTempPath.toString())
        }
    }

    func getTarDirName(tarFilePath: String): String {
        var tarFileName = tarFilePath.replace("\\", "/")
        tarFileName = tarFileName.split("/")[tarFileName.split("/").size - 1]
        return tarFilePath.removeSuffix(tarFileName)
    }
}
