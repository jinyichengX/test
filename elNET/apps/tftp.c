udp_t * u_s = NULL;
net_err_t tftp_server_init(void)
{
    u_s = udp_create();
    if( u_s == NULL )
        goto _error;

    /* tftp server usually bind to 0.0.0.0 */
    endpoint_t p = {.ip4addr.ipv = 0, .port = TFTP_PORT};
    if(NET_ERR_NOK == udp_bind(u_s, &p))
        goto _error;
    return NET_ERR_OK;
_error:
    if( u_s ) udp_destroy(u_s);
    return NET_ERR_NOK;
}

net_err_t tftp_server_start(void)
{
    ip4addr_t from;
    uint16_t port, len;
    static char server_buffer[600];
    tftp_hdr_t * hdr;
    while(1)
    {    
        if(NET_ERR_OK != udp_recvfrom(u_s, server_buffer, 600, &from, &port, &len))
            break;
        hdr = (tftp_hdr_t *)server_buffer;
        /* parse request */
        switch(_htons(hdr->opcode))
        {
        case TFTP_OPCODE_READ_REQ: 
            break;
        case TFTP_OPCODE_WRITE_REQ: 
            break;
        case TFTP_OPCODE_DATA: 
            break;
        case TFTP_OPCODE_ACK: 
            break;
        case TFTP_OPCODE_ERROR: 
            break;
        default:
            /* not such code err return */
            break;
        }
    }
    return NET_ERR_NOK;
}