using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PatchManifestClient
{
    // Config.Json
    public class ConfigJsonInfo
    {
        public bool bConfigSave { get; set; }
        public int PlatformIdx { get; set; }    
        public string? EmbeddedPath { get; set; }
        public string? BuildPath { get; set; }
        public string? FileVersionPath { get; set; }
        public string? FileExportPath { get; set; }
    }

    // 단일 Pak 내용
    public class PakFileInfo
    {
        public string? PakName { get; set; }
        public Int64 PakBytes { get; set; }
        public string? PakVersion { get; set; }
        public Int32 PakChunkId { get; set; }
        public string? PakRelativeUrl { get; set; }
        public string? PakType { get; set; }
    }

    // ServerManifest.Json
    public class ServerManifestJsonInfo
    {
        public string? FileVersion { get; set; }
        public string? ContentPlatform { get; set; }
        public string? ContentVersion { get; set; }
        public Int32 PakCount { get; set; }
        public IList<PakFileInfo>? PakFiles { get; set; }
        public string? Description { get; set; }
    }

}
