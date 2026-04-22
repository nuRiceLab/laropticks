std::shared_ptr<ROOT::RGeomViewer> rgeom_viewer;

void viewer()
{
   // geometry is too large, please provide import like:
   // TGeoManager::Import("filename.root");
   
   rgeom_viewer = std::make_shared<ROOT::RGeomViewer>(gGeoManager, "volWorld");
   rgeom_viewer->Description().SetVisLevel(5);
   rgeom_viewer->Description().SetMaxVisFaces(100000);
   rgeom_viewer->Description().SetNSegments(20);
   rgeom_viewer->Description().SetJsonComp(103);
   rgeom_viewer->SetShowHierarchy(true);
   rgeom_viewer->SetShowColumns(true);
   
   rgeom_viewer->Show();
}
