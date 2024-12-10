//custom.js
function _isCoordInPolygon(lat,lng,arrayLatLng){
    //判断点在多边形内
    //https://www.cnblogs.com/muyefeiwu/p/11260366.html
    var ncross = 0;
	for (var i = 0; i < arrayLatLng.length; i++)
	{
		var p1 = arrayLatLng[i];
		var p2 = arrayLatLng[(i + 1) % arrayLatLng.length]; //相邻两条边p1,p2
		if (p1[0] == p2[0]){
			continue;
		}
		if (lat < Math.min(p1[0], p2[0])){
			continue;
		}
		if (lat >= Math.max(p1[0], p2[0])){
			continue;
		}
		var _lng = (lat - p1[0])*(p2[1] - p1[1]) / (p2[0] - p1[0]) + p1[1];
		if (_lng > lng)
			ncross++; //只统计单边交点
	}
	return(ncross % 2 == 1);
}

function calcMaxInt(magnitude,depth,epicenterLng,epicenterLat){
    const params={
        default:{a:3.944,b:1.071,c:1.2355678010148,d:7},
        west:{a:3.6113,b:1.4347,c:1.6710348780191,d:13},
        xinjiang:{a:3.3682,b:1.2746,c:1.4383398946154,d:9},
        neijiangYibin:{a:3.6588,b:1.3626,c:1.5376630426267,d:13}
    };
    const polygonNeijiangYibin=[
        [30.06,104.45],
        [28.61,103.61],
        [27.85,104.57],
        [28.15,105.35],
        [29.69,105.41]
    ];
    const polygonChuanZangQing=[
        [39.15,97.02],
        [34.01,78.57],
        [31.11,78.39],
        [26.51,92.40],
        [26.35,103.95],
        [31.95,108.42],
        [33.54,102.47]
    ];
    const polygonXinjiang=[
        [47.20,82.91],
        [40.47,72.43],
        [32.81,76.56],
        [37.23,92.24],
        [45.92,92.07]
    ];
    //根据地点选择参数
    let chosenParams;
    if(_isCoordInPolygon(epicenterLat,epicenterLng,polygonNeijiangYibin)){
        //四川内江、宜宾
        chosenParams=params.neijiangYibin;
    }else if(_isCoordInPolygon(epicenterLat,epicenterLng,polygonChuanZangQing)){
        //四川其他地区、西藏、青海
        chosenParams=params.west;
    }else if(_isCoordInPolygon(epicenterLat,epicenterLng,polygonXinjiang)){
        //新疆
        chosenParams=params.xinjiang;
    }else{
        //其他
        chosenParams=params.default;
    }
    const {a,b,c,d}=chosenParams;
    const maxInt=a+b*magnitude-c*Math.log(d*(depth+25)/40)+0.2;
    return Math.round(maxInt);
}
