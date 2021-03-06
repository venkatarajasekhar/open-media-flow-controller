/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 14170 $ of $
 *
 * $Id:$
 */
#ifndef JMFCNAMESPACEHTTPCLIENTTABLE_DATA_ACCESS_H
#define JMFCNAMESPACEHTTPCLIENTTABLE_DATA_ACCESS_H

#ifdef __cplusplus
extern "C" {
#endif


/* *********************************************************************
 * function declarations
 */

/* *********************************************************************
 * Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table jmfcNamespaceHttpClientTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * JUNIPER-MFC-MIB::jmfcNamespaceHttpClientTable is subid 2 of jmfcNamespace.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.2636.1.2.1.4.2, length: 12
*/


    int jmfcNamespaceHttpClientTable_init_data(jmfcNamespaceHttpClientTable_registration * jmfcNamespaceHttpClientTable_reg);


    /*
     * TODO:180:o: Review jmfcNamespaceHttpClientTable cache timeout.
     * The number of seconds before the cache times out
     */
#define JMFCNAMESPACEHTTPCLIENTTABLE_CACHE_TIMEOUT   60

void jmfcNamespaceHttpClientTable_container_init(netsnmp_container **container_ptr_ptr,
                             netsnmp_cache *cache);
void jmfcNamespaceHttpClientTable_container_shutdown(netsnmp_container *container_ptr);

int jmfcNamespaceHttpClientTable_container_load(netsnmp_container *container);
void jmfcNamespaceHttpClientTable_container_free(netsnmp_container *container);

int jmfcNamespaceHttpClientTable_cache_load(netsnmp_container *container);
void jmfcNamespaceHttpClientTable_cache_free(netsnmp_container *container);

    /*
    ***************************************************
    ***             START EXAMPLE CODE              ***
    ***---------------------------------------------***/
/* *********************************************************************
 * Since we have no idea how you really access your data, we'll go with
 * a worst case example: a flat text file.
 */
#define MAX_LINE_SIZE 256
    /*
    ***---------------------------------------------***
    ***              END  EXAMPLE CODE              ***
    ***************************************************/
    int jmfcNamespaceHttpClientTable_row_prep( jmfcNamespaceHttpClientTable_rowreq_ctx *rowreq_ctx);



#ifdef __cplusplus
}
#endif

#endif /* JMFCNAMESPACEHTTPCLIENTTABLE_DATA_ACCESS_H */
