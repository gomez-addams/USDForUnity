#pragma once

namespace usdi {

class Context
{
public:
    Context();
    virtual ~Context();

    bool                valid() const;
    void                initialize();
    bool                createStage(const char *identifier);
    bool                open(const char *path);
    bool                save();
    // path must *not* be same as identifier (parameter of createStage() or open())
    bool                saveAs(const char *path);

    const ImportConfig& getImportConfig() const;
    void                setImportConfig(const ImportConfig& v);
    const ExportConfig& getExportConfig() const;
    void                setExportConfig(const ExportConfig& v);

    Schema*             getRootSchema();
    Schema*             findSchemaByPath(const char *path);

    UsdStageRefPtr      getUSDStage() const;
    int                 generateID();

    void                updateAllSamples(Time t);
    void                invalidateAllSamples();

    // T: Xform, Camera, Mesh, ...
    template<class T> T* createSchema(Schema *parent, const char *name);
    template<class T> T* createSchema(Schema *parent, const UsdPrim& t);
    Schema*              createSchema(Schema *parent, UsdPrim prim);
    Schema*              createSchemaRecursive(Schema *parent, UsdPrim prim);
    Schema*              createReferenceSchemaRecursive(Schema *parent, UsdPrim prim);
    Schema*              createReference(const char *dstprim, const char *assetpath, const char *srcprim);
    void                flatten();

private:
    void    addSchema(Schema *schema);
    void    applyImportConfig();

private:
    typedef std::map<std::string, std::string> Variants;
    typedef std::unique_ptr<Schema> SchemaPtr;
    typedef std::vector<SchemaPtr> Schemas;

    UsdStageRefPtr  m_stage;
    Schemas         m_schemas;
    std::string     m_prim_root;
    Variants        m_variants;

    ImportConfig    m_import_config;
    ExportConfig    m_export_config;

    int             m_id_seed = 0;
    double          m_start_time = 0.0;
    double          m_end_time = 0.0;
};

} // namespace usdi
