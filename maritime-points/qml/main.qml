
// Copyright 2016 ESRI
//
// All rights reserved under the copyright laws of the United States
// and applicable international laws, treaties, and conventions.
//
// You may freely redistribute and use this sample code, with or
// without modification, provided you include the original copyright
// notice and use restrictions.
//
// See the Sample code usage restrictions document for further information.
//

import QtQuick 2.6
import QtQuick.Controls 1.4
import Esri.ArcGISRuntime 100.2
import Maritime 1.0;

ApplicationWindow {
    id: appWindow
    width: 800
    height: 600
    title: "Maritime-points"

    property string nmeaPath: ":/Resources/nmea-sample"

    Rectangle {
        id: rootRectangle
        clip: true
        width: 800
        height: 600

        MapView {
            id: mapView
            anchors.fill: parent

            Map {
                BasemapOceans {}

                // Declare a FeatureCollectionLayer
                FeatureCollectionLayer {

                    FeatureCollection
                    {
                        // Create a Point FeatureCollectionTable inside the FeatureCollection
                        FeatureCollectionTable {
                            id: pointsTable

                            // define the schema of the table
                            geometryType: Enums.GeometryTypePoint
                            spatialReference: SpatialReference { wkid: 4326 }
                            Field {
                                                        id: placeField
                                                        alias: "Place Name"
                                                        name: "Place"
                                                        length: 50
                                                        fieldType: Enums.FieldTypeText
                                                    }

                            // define the renderer
                                                 SimpleRenderer {
                                                     SimpleMarkerSymbol {
                                                         style: Enums.SimpleMarkerSymbolStyleTriangle
                                                         color: "red"
                                                         size: 18
                                                     }
                                                 }

                            Component.onCompleted: {
                                MaritimeTableFactory.loadAIS(pointsTable, nmeaPath);
                            }
                        }
                    }
                }
            }
        }
    }
}
