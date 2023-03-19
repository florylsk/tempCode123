Add-Type -Name "Netapi32" -Namespace Win32 -MemberDefinition '
[DllImport("Netapi32.dll", SetLastError = true)]
public static extern int NetApiBufferFree(IntPtr Buffer);

[DllImport("Netapi32.dll", CharSet = CharSet.Unicode)]
public static extern int NetShareEnum(
    StringBuilder ServerName,
    int level,
    ref IntPtr bufPtr,
    uint prefmaxlen,
    ref int entriesread,
    ref int totalentries,
    ref int resume_handle
);

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
public struct SHARE_INFO_1
{
    public string shi1_netname;
    public uint shi1_type;
    public string shi1_remark;

    public SHARE_INFO_1(string sharename, uint sharetype, string remark)
    {
        this.shi1_netname = sharename;
        this.shi1_type = sharetype;
        this.shi1_remark = remark;
    }

    public override string ToString()
    {
        return shi1_netname;
    }
}'

function Get-NetShares {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ServerName
    )

    $ShareInfos = New-Object System.Collections.Generic.List[Win32.Netapi32+SHARE_INFO_1]
    $entriesread = 0
    $totalentries = 0
    $resume_handle = 0
    $nStructSize = [System.Runtime.InteropServices.Marshal]::SizeOf([Win32.Netapi32+SHARE_INFO_1])
    $bufPtr = [System.IntPtr]::Zero
    $server = New-Object System.Text.StringBuilder($ServerName)
    $ret = [Win32.Netapi32]::NetShareEnum($server, 1, [ref]$bufPtr, 0xFFFFFFFF, [ref]$entriesread, [ref]$totalentries, [ref]$resume_handle)

    if ($ret -eq [Win32.Netapi32+NetError]::NERR_Success) {
        $currentPtr = $bufPtr
        for ($i = 0; $i -lt $entriesread; $i++) {
            $shi1 = [System.Runtime.InteropServices.Marshal]::PtrToStructure($currentPtr, [Win32.Netapi32+SHARE_INFO_1])
            $ShareInfos.Add($shi1)
            $currentPtr = [System.IntPtr]($currentPtr.ToInt32() + $nStructSize)
        }
        [Win32.Netapi32]::NetApiBufferFree($bufPtr) | Out-Null
        return $ShareInfos.ToArray()
    }
    else {
        $ShareInfos.Add((New-Object Win32.Netapi32+SHARE_INFO_1("ERROR=$ret", 10, [string]::Empty)))
        return $ShareInfos.ToArray()
    }
}
