#only computers
$objSearcher=[adsisearcher]'(&(objectCategory=computer))'
$objSearcher.PageSize = 200

#specify properties to include
$colProplist = "name"
foreach ($i in $colPropList) { $objSearcher.PropertiesToLoad.Add($i) | out-null } 
	
$colResults = $objSearcher.FindAll()

foreach ($objResult in $colResults)
{
    #computer name
    $assetname = ($objResult.Properties).name    
    write-host $assetname  		
}
